#pragma once
#include "EasySilkContext.h"
#include <QFile>

/**
 * @brief Silk相关解码器操作
 * @author 龚建波
 * @date 2021-03-31
 * @details
 * SDK地址：https://gitee.com/alvis/SILK_SDK_SRC
 * 参照示例：SILK_SDK_SRC_v1.0.9\SILK_SDK_SRC_ARM_v1.0.9\test\Decoder.c
 * 参照博客：https://blog.csdn.net/weixin_34292924/article/details/87987436
 * 采样率自适应
 * 参考博客：https://blog.csdn.net/zhaosipei/article/details/7467810
 */
class EasySilkDecoder final : public EasyAbstractDecoder
{
public:
    EasySilkDecoder();
    ~EasySilkDecoder();

    //绑定一个上下文进行处理
    void setContext(const QSharedPointer<EasySilkContext> &contextPtr);

    //是否为有效的音频文件（该解码器可解析）
    //判断为有效时才进行后续操作
    bool isValid() const override;
    //开始解码
    //format:转出参数
    //return false:表示无法转换
    bool open(const QAudioFormat &format) override;
    //转码所有数据
    //转码失败则返回数据为空
    QByteArray readAll() override;
    //转码数据
    //size:期望返回的数据的最大长度
    //转码失败或者结束则返回数据为空
    QByteArray read(qint64 maxSize) override;
    //结束解码
    void close() override;

private:
    //这些变量是从示例拿的
    static constexpr int MAX_BYTES_PER_FRAME = 1024;
    static constexpr int MAX_INPUT_FRAMES = 5;
    static constexpr int MAX_FRAME_LENGTH = 480;
    static constexpr int FRAME_LENGTH_MS = 20;
    static constexpr int MAX_API_FS_KHZ = 48;

    QSharedPointer<EasySilkContext> theContextPtr;
    //文件
    QFile decodeFile;
    //转出的采样率
    int decodeSampleRate;
    //上一次read多出maxSize的数据
    QByteArray dataTemp;

    //解码参数
    SKP_SILK_SDK_DecControlStruct dec_ctrl;
    QVector<SKP_uint8> dec_state;
    //默认解出来貌似是16bit的精度
    SKP_uint8 payload[MAX_BYTES_PER_FRAME * MAX_INPUT_FRAMES];
    SKP_uint8 *payload_ptr = NULL;
    SKP_int16 out[((FRAME_LENGTH_MS * MAX_API_FS_KHZ) << 1) * MAX_INPUT_FRAMES];
    SKP_int16 *out_ptr = NULL;
};

