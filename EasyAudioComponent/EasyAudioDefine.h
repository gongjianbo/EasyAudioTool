#pragma once
//本文件用于定义数据结构及全局变量等
#include <QMetaObject>
#include <QAudioFormat>
#include <QSharedPointer>
#include <atomic>
#include <memory>
#include <cmath>
#include <algorithm>
//在头文件导入只是偷个懒
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswresample/swresample.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
}

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
 * @brief wav-pcm文件头结构体
 * @author 龚建波
 * @date 2020-11-12
 * @details
 * 1.wav头是不定长格式，为了简化操作这里用的固定44字节长格式
 * 2.数值以小端存储，不过pc一般是小端存储，暂不特殊处理
 * 3.参照列表
 * 参照：https://www.cnblogs.com/ranson7zop/p/7657874.html
 * 参照：https://www.cnblogs.com/Ph-one/p/6839892.html
 * 参照：https://blog.csdn.net/imxiangzi/article/details/80265978
 * @note
 * 1.最大元素为4字节，所以本身就是44字节对齐的
 * 加字节对齐只是为了提醒
 * 2.pcm导出为wav时使用，解析时慎用
 * @history
 * 2021-03-30 更新注释
 */
#pragma pack(push,1)
struct EasyWavHead
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

    //根据设置生成wav(pcm)文件头信息
    //params:参数信息
    //dataSize:pcm数据字节长度
    //return EasyAudioWavHead: wav头
    static EasyWavHead createHead(const QAudioFormat &params,unsigned int dataSize);
};
#pragma pack(pop)
Q_DECLARE_METATYPE(EasyWavHead)

/**
 * @brief 放枚举或者全局变量
 * @author 龚建波
 * @date 2021-
 */
class EasyAudio : public QObject
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
public:
    //QAudioFormat ffmpegToQtFormat(int channels, int sampleRate, AVSampleFormat sampleFmt);
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
struct EasyAudioInfo
{
    //文件路径
    QString filepath;
    //文件名
    QString filename;
    //文件大小:byte
    qint64 filesize;
    //格式，如wav
    QString format;
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

    //打印参数信息
    void dumpAudioInfo();
};
Q_DECLARE_METATYPE(EasyAudioInfo)
