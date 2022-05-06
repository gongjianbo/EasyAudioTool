#pragma once
#include "EasyFFmpegContext.h"

/**
 * @brief FFmpeg相关解码器操作
 * @author 龚建波
 * @date 2021-03-30
 * @details
 * 1.主要功能为将音频数据解码为PCM
 * 2.本类只处理编解码，不带多线程处理部分
 * 3.转换采用libswresample，该模块主要功能为采样率-声道-样本格式转换等
 *
 * (2021-8-28移除了部分无效的地址)
 * 测试文件地址：
 * https://samples.mplayerhq.hu/A-codecs/
 * 音乐下载：
 * http://www.musictool.top/
 * 音频读取参考：
 * ffmpeg-4.2.4\doc\examples\filtering_audio.c
 * 重采样参考：
 * ffmpeg-4.2.4\doc\examples\resampling_audio.c
 * https://www.jianshu.com/p/bf5e54f553a4
 * https://blog.csdn.net/zhuweigangzwg/article/details/53395009
 */
class EASYAUDIOTOOL_EXPORT EasyFFmpegDecoder final : public EasyAbstractDecoder
{
public:
    EasyFFmpegDecoder();
    ~EasyFFmpegDecoder();

    //绑定一个上下文进行处理
    void setContext(const QSharedPointer<EasyAbstractContext> &contextPtr) override;

    //是否为有效的音频文件（该解码器可解析）
    //判断为有效时才进行后续操作
    bool isValid() const override;
    //开始解码
    //format:转出参数
    //return false:表示无法转换
    bool open(const QAudioFormat &format) override;
    //重置读取位置到开始
    //return false:表示无法跳转
    bool reset() override;
    //转码数据，每次调用都从上一次读取的位置继续读，直到结束
    //outBuffer:转码后输出缓冲区
    //maxSize:期望返回的数据的最大长度
    //返回实际读取数据的长度，<=maxSize，如果读取失败返回-1
    qint64 read(char *outBuffer, qint64 maxSize) override;
    //转码所有数据
    //考虑到解码不便于重入，所以独立于read实现
    //callBack:转换时的同步回调函数
    //  每次packet处理都会调用，若返回false则整个toPcm无效返回false
    //  回调函数<参数1>为输出缓冲区地址，<参数2>为输出数据有效字节长度
    //  （TODO 数据的左右声道也可以在这里区分）
    //返回实际读取数据的总长度，如果读取失败返回-1
    qint64 readAll(std::function<bool (const char *, qint64)> callBack = nullptr) override;
    //结束解码
    void close() override;

private:
    void resetOutBuffer(int bufferSize);

private:
    QSharedPointer<EasyFFmpegContext> theContextPtr;
    //输入数据参数
    EasyFFmpegContext::ArgFormat inFormat;
    //转换转出参数
    EasyFFmpegContext::ArgFormat outFormat;
    //上一次read多出maxSize的数据
    QByteArray dataTemp;

    //下面这些本来在解码函数中，现在拿出来配合解析流
    //描述存储压缩数据
    //视频通常包含一个压缩帧，音频可能包含多个压缩帧
    AVPacket *packet{ NULL };
    //描述原始数据
    AVFrame *frame{ NULL };
    //重采样上下文
    SwrContext *swr_ctx{ NULL };
    //解析时out缓冲，单个通道初始1M+大小
    int out_bufsize{ 1024*1024*2 };
    //out_buffer为缓冲区
    uint8_t *out_buffer{ NULL };
    //out_buffer_arr保存了out_buffer和out_buffer+len/2两个地址
    //用于双声道数据获取参数
    uint8_t *out_buffer_arr[2]{ NULL, NULL};
};
