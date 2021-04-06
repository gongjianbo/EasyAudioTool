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
 * 测试文件地址：
 * https://samples.mplayerhq.hu/A-codecs/
 * 音乐下载：
 * http://www.musictool.top/
 * 重采样参考：
 * ffmpeg-4.2.4\doc\examples\resampling_audio.c
 * https://www.jianshu.com/p/bf5e54f553a4
 * https://segmentfault.com/a/1190000025145553
 * https://blog.csdn.net/bixinwei22/article/details/86545497
 * https://blog.csdn.net/zhuweigangzwg/article/details/53395009
 */
class EasyFFmpegDecoder final : public EasyAbstractDecoder
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
    //转码所有数据
    //callBack:转换时的同步回调函数
    //如果设置了回调则无返回数据
    //转码失败返回数据为空
    QByteArray readAll(std::function<bool (const char *, int)> callBack =
            std::function<bool (const char *, int)>()) override;
    //转码数据
    //size:期望返回的数据的最大长度
    //转码失败或者结束则返回数据为空
    QByteArray read(qint64 maxSize) override;
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
    AVPacket *packet = NULL;
    //描述原始数据
    AVFrame *frame = NULL;
    //重采样上下文
    SwrContext *swr_ctx = NULL;
    //解析时out缓冲，单个通道初始1M+大小
    int out_bufsize = 1024*1024*2;
    //out_buffer为缓冲区
    uint8_t *out_buffer = NULL;
    //out_buffer_arr保存了out_buffer和out_buffer+len/2两个地址
    //用于双声道数据获取参数
    uint8_t *out_buffer_arr[2] = { NULL, NULL};
};

