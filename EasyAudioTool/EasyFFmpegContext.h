#pragma once
#include "EasyAudioCommon.h"
#include "EasyAudioInterface.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswresample/swresample.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
}

/**
 * @brief FFmpeg库相关上下文
 * @author 龚建波
 * @date 2021-03-30
 * @details
 * 1.去掉了拷贝和赋值，需要作为参数传递时请使用智能指针管理
 * 2.空指针主要用 NULL，为了和 C 接口保持一致
 *
 * 内存管理参考：
 * https://www.jianshu.com/p/9f45d283d904
 * https://blog.csdn.net/leixiaohua1020/article/details/41181155
 * 获取音频信息参考：
 * https://blog.csdn.net/zhoubotong2012/article/details/79340722
 * https://blog.csdn.net/luotuo44/article/details/54981809
 *
 * @todo
 * 内存泄露测试
 */
class EASYAUDIOTOOL_EXPORT EasyFFmpegContext final : public EasyAbstractContext
{
public:
    //参数格式，默认单声道、16K、signed short存储
    struct ArgFormat
    {
        int channels = 1;
        int sampleRate = 16000;
        AVSampleFormat sampleFmt = AV_SAMPLE_FMT_S16;
        //sampleFmt对应的字节数
        int sampleByte = 2;
    };
public:
    explicit EasyFFmpegContext(const QString &filepath);
    ~EasyFFmpegContext();

    //获取音频格式等信息
    EasyAudioInfo audioInfo() const override;
    //重采样时需要源数据的通道、采样率、精度信息
    EasyFFmpegContext::ArgFormat audioFormat() const;

    //QAudioFormat转为ArgFormat
    static EasyFFmpegContext::ArgFormat getFormat(const QAudioFormat &qtformat);

private:
    //初始化与释放
    void initContext(const QString &filepath);
    void freeContext();
    //获取QuickTime / MOV格式的具体格式信息(mov,mp4,m4a,3gp,3g2,mj2)
    QString getMovFormat() const;

private:
    //音频源文件路径
    QString audiopath;

    //格式化I/O上下文
    AVFormatContext *formatCtx{ NULL };
    //解码器
    AVCodec *codec{ NULL };
    //解码器上下文
    AVCodecContext *codecCtx{ NULL };
    //音频流index
    int audioStreamIndex{ -1 };

    //暂存音频信息，便于重复读取
    mutable bool hasInfoTemp{ false };
    mutable EasyAudioInfo infoTemp;

    friend class EasyFFmpegDecoder;
    Q_DISABLE_COPY_MOVE(EasyFFmpegContext)
};
