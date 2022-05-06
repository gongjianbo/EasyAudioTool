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
 * SILK格式采样率自适应
 * 参考博客：https://blog.csdn.net/zhaosipei/article/details/7467810
 */
class EASYAUDIOTOOL_EXPORT EasySilkDecoder final : public EasyAbstractDecoder
{
public:
    EasySilkDecoder();
    ~EasySilkDecoder();

    //绑定一个上下文进行处理，在oepn之前设置
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
    //这些变量是从示例拿的
    static constexpr int MAX_BYTES_PER_FRAME{ 1024 };
    static constexpr int MAX_INPUT_FRAMES{ 5 };
    static constexpr int MAX_FRAME_LENGTH{ 480 };
    static constexpr int FRAME_LENGTH_MS{ 20 };
    static constexpr int MAX_API_FS_KHZ{ 48 };

    QSharedPointer<EasySilkContext> theContextPtr;
    //文件读写
    QFile decodeFile;
    //reset跳过格式头长度，偏移到数据位置
    int dataOffset{ 0 };
    //转出的采样率
    int decodeSampleRate;
    //上一次read多出maxSize的数据
    QByteArray dataTemp;

    //解码参数
    SKP_SILK_SDK_DecControlStruct dec_ctrl;
    QVector<SKP_uint8> dec_state;
    //默认解出来貌似是16bit的精度
    SKP_uint8 payload[MAX_BYTES_PER_FRAME * MAX_INPUT_FRAMES];
    SKP_uint8 *payload_ptr{ NULL };
    SKP_int16 out[((FRAME_LENGTH_MS * MAX_API_FS_KHZ) << 1) * MAX_INPUT_FRAMES];
    SKP_int16 *out_ptr{ NULL };
};
