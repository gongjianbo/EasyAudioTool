#pragma once
#include "EasyAudioCommon.h"
#include "EasyAudioInterface.h"
#include "SKP_Silk_SDK_API.h"

/**
 * @brief Silk库相关上下文
 * @author 龚建波
 * @date 2021-03-31
 * @details
 * 1.DecControlStruct只看到采样率设置，精度默认16bit通道默认1
 * 采样率可选项4种：8000,12000,16000,24000
 */
class EASYAUDIOTOOL_EXPORT EasySilkContext final : public EasyAbstractContext
{
public:
    explicit EasySilkContext(const QString &filepath);
    ~EasySilkContext();

    //获取音频格式等信息
    EasyAudioInfo audioInfo() const override;

    //判断是否为silk格式
    static bool isSilkAudio(const QString &filepath);
    //解码slik数据
    //（参数只有采样率，采样深度固定16bit，通道固定1）
    //（要获取时长得解完整个数据包，所以把解码函数从decoder挪过来的）
    //filepath:文件路径
    //params:目标格式的参数，如果参数无效会使用原数据参数
    //callBack:转换时的同步回调函数
    // 每次packet处理都会调用，若返回false则整个toPcm无效返回false
    // 回调函数参描1为输出缓冲区地址，参数2为输出数据有效字节长度
    //return false:表示转换无效失败
    static bool toPcm(const QString &filepath,
                      int sampleRate,
                      std::function<bool(const char *outData, int outSize)> callBack);

private:
    //初始化与释放
    void initContext(const QString &filepath);
    void freeContext();

private:
    //音频源文件路径
    QString audiopath;
    //暂存音频信息，便于重复读取
    mutable bool hasInfoTemp{ false };
    mutable EasyAudioInfo infoTemp;

    friend class EasySilkDecoder;
    Q_DISABLE_COPY_MOVE(EasySilkContext)
};
