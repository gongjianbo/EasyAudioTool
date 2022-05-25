#pragma once
//本文件用于定义数据结构及全局变量等
#include <QtCore/qglobal.h>
#include <QMetaObject>
#include <QAudioFormat>
#include <QSharedPointer>
#include <atomic>
#include <memory>
#include <cmath>
#include <algorithm>
#include <functional>

//导入导出符号
#if defined(EASYAUDIOTOOL_LIBRARY)
#  define EASYAUDIOTOOL_EXPORT Q_DECL_EXPORT
#else
#  define EASYAUDIOTOOL_EXPORT Q_DECL_IMPORT
#endif

//[]在上一版wav-pcm转换参数使用了自定义类型，现采用QAudioFormat
//struct SimpleAudioParameter
//{
//    int channels=1;
//    AVSampleFormat sampleFormat=AV_SAMPLE_FMT_S16;
//    int sampleRate=16000;
//};
//目前主要参数：采样率 采样精度 通道数
//Qt和ffmpeg的采样存储格式有些不匹配
//Qt      QAudioFormat(setSampleSize(16),setSampleType(QAudioFormat::SignedInt))
//ffmpeg  AVSampleFormat(=AV_SAMPLE_FMT_S16)

/**
 * @brief wav文件头结构体
 * @author 龚建波
 * @date 2020-11-12
 * @details
 * wav格式头是不定长的，不过这里用的比较简单的固定格式，便于处理
 * 包含RIFF块、文件格式标志、fmt块、压缩编码时fact块、data块
 * （数值以小端存储，不过pc一般默认小端存储，暂不特殊处理）
 * 参照：https://www.cnblogs.com/ranson7zop/p/7657874.html
 * 参照：https://www.cnblogs.com/Ph-one/p/6839892.html
 * 参照：http://mobile.soomal.com/doc/10100001099.htm
 * @todo
 * 划分为RIFF和RF64，以支持更长的数据内容
 * @note
 * 此处wav/wave指Microsoft Wave音频文件格式，后缀为".wav"
 * wave长度变量为uint32四个字节，只支持到4GB（2^32）
 * （网上有说只支持2G，用工具拼接实测是可以支持到接近4GB的）
 * 对于更长的数据，可以使用Sony Wave64，后缀为".w64"；
 * 或者使用RF64，RF64即RIFF的64bit版本。
 * Adobe Audition软件保存wav时如果数据超过4GB，会使用RF64格式保存
 */
#pragma pack(push,1)
struct EASYAUDIOTOOL_EXPORT EasyWavHead
{
    char riffFlag[4]; //文档标识，大写"RIFF"
    //从下一个字段首地址开始到文件末尾的总字节数。
    //该字段的数值加 8 为当前文件的实际长度。
    unsigned int riffSize; //数据长度
    char waveFlag[4]; //文件格式标识，大写"WAVE"
    char fmtFlag[4]; //格式块标识，小写"fmt "
    unsigned int fmtSize; //格式块长度，可以是 16、 18 、20、40 等
    unsigned short compressionCode; //编码格式代码，1为pcm
    unsigned short numChannels; //通道个数
    unsigned int sampleRate; //采样频率
    //该数值为:声道数×采样频率×每样本的数据位数/8。
    //播放软件利用此值可以估计缓冲区的大小。
    unsigned int bytesPerSecond; //码率（数据传输速率）
    //采样帧大小。该数值为:声道数×位数/8。
    //播放软件需要一次处理多个该值大小的字节数据,用该数值调整缓冲区。
    unsigned short blockAlign; //数据块对其单位
    //存储每个采样值所用的二进制数位数。常见的位数有 4、8、12、16、24、32
    unsigned short bitsPerSample; //采样位数（采样深度）
    char dataFlag[4]; //表示数据开头，小写"data"
    unsigned int dataSize; //数据部分的长度

    //默认构造
    EasyWavHead();

    /**
     * @brief 根据采样率、精度等参数构造wav格式头
     * @param format Qt音频格式类，包含有采样率、精度等信息
     * @param dataSize 有效数据字节数
     */
    EasyWavHead(const QAudioFormat &format, unsigned int dataSize);

    /**
     * @brief 根据采样率、精度等参数构造wav格式头
     * @param sampleRate 采样率，如16000Hz
     * @param sampleSize 采样精度，如16位
     * @param channelCount 声道数，如1单声道
     * @param dataSize 有效数据字节数
     */
    EasyWavHead(int sampleRate, int sampleSize,
                int channelCount, unsigned int dataSize);

    /**
     * @brief 判断该wav头参数是否有效
     * 主要用在读取并解析使用该头格式写的文件
     * @return =true则格式有效
     */
    bool isValid() const;
};
#pragma pack(pop)
Q_DECLARE_METATYPE(EasyWavHead)

/**
 * @brief 放枚举或者全局变量
 * @author 龚建波
 * @date 2021-
 */
class EASYAUDIOTOOL_EXPORT EasyAudio : public QObject
{
    Q_OBJECT
public:
    //编解码器类型
    //每一种解码器对应相应的Context和Decode/Encode类
    enum CodecType
    {
        None
        ,FFmpeg
        ,SilkV3
        //,Sndfile
        //,Libav
    };
    Q_ENUM(CodecType)

    //播放状态
    enum PlayerState
    {
        Stopped //未操作，停止状态
        ,Paused //暂停
        ,Playing //播放
    };
    Q_ENUM(PlayerState)
};

/**
 * @brief 识别到的音频文件信息
 * @author 龚建波
 * @date 2021-03-30
 * @details
 * 1.成员参数为负数时，表示自适应参数或者无效参数
 * （如silk自适应采样率）
 * 2.移除了上个版本一些和音频信息无关的，如转换前的源文件信息
 * 此功能可通过增加一层wrapper
 */
struct EASYAUDIOTOOL_EXPORT EasyAudioInfo
{
    //文件路径
    QString filepath;
    //文件名
    QString filename;
    //文件大小:byte
    qint64 filesize;
    //文件md5值，此项不是必要的，可移除
    QString md5;
    //格式，如wav
    QString format;
    //编解码器，如FFmpeg等
    QString codec;
    //编码，如pcm_s16
    QString encode;
    //通道数
    int channels;
    //采样率:hz
    //pcm时为每秒每个通道采样个数
    int sampleRate;
    //采样精度:bit
    //s16则以signed short存储一个采样
    int sampleBit;
    //比特率:bps
    //比特率=采样率*单个的周期音频数据长度
    //pcm时16bit双声道48KHz音频的比特率：
    //48KHz * 16 * 2 = 1536kbps
    qint64 bitRate;
    //音频时长，毫秒:ms
    qint64 duration;

    //有效性标识，默认false
    bool valid{ false };

    //打印参数信息
    void dumpAudioInfo();
};
Q_DECLARE_METATYPE(EasyAudioInfo)
