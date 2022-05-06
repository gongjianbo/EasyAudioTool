#pragma once
//本文件用于放置接口定义
#include <QObject>
#include "EasyAudioCommon.h"

/**
 * @brief 编解码上下文基类
 * @author 龚建波
 * @date 2021-03-30
 */
class EASYAUDIOTOOL_EXPORT EasyAbstractContext
{
public:
    explicit EasyAbstractContext(EasyAudio::CodecType codecType);
    virtual ~EasyAbstractContext() = default;

    //init/free是否有必要做为虚函数？
    //解析
    //virtual void initContext(const QString &filepath) = 0;
    //释放
    //virtual void freeContext() = 0;
    //获取音频格式等信息
    virtual EasyAudioInfo audioInfo() const = 0;

    //是否为有效的上下文
    bool isValid() const;
    //编解码器类型
    EasyAudio::CodecType codecType() const;
    //获取错误信息
    QString lastError() const;

protected:
    void setValid(bool valid);
    void setLastError(const QString &info);

private:
    //是否为有效的上下文
    bool theValid = false;
    //编解码器类型
    EasyAudio::CodecType theCodecType;
    //错误信息，不同解码器状态码有区别，所以用字符串保存错误信息
    QString theError;

    Q_DISABLE_COPY_MOVE(EasyAbstractContext)
};

/**
 * @brief 解码器基类
 * @author 龚建波
 * @date 2021-03-30
 * @note
 * 2022-05-06 接口参照QIODevice进行了修改
 */
class EASYAUDIOTOOL_EXPORT EasyAbstractDecoder
{
public:
    EasyAbstractDecoder() = default;
    virtual ~EasyAbstractDecoder() = default;

    //绑定一个上下文进行处理，在oepn之前设置
    virtual void setContext(const QSharedPointer<EasyAbstractContext> &contextPtr) = 0;
    //是否为有效的音频文件（该解码器可解析）
    //判断为有效时才进行后续操作
    virtual bool isValid() const = 0;
    //开始解码
    //format:转出参数
    //return false:表示无法转换
    virtual bool open(const QAudioFormat &format) = 0;
    //重置读取位置到开始
    //return false:表示无法跳转
    virtual bool reset() = 0;
    //转码数据，每次调用都从上一次读取的位置继续读，直到结束
    //outBuffer:转码后输出缓冲区
    //maxSize:期望返回的数据的最大长度
    //返回实际读取数据的长度，<=maxSize，如果读取失败返回-1
    virtual qint64 read(char *outBuffer, qint64 maxSize) = 0;
    //读取数据
    //maxSize:期望返回的数据的最大长度
    //数据无效或已读取完则返回空
    QByteArray read(qint64 maxSize);
    //转码所有数据
    //考虑到解码不便于重入，所以独立于read实现
    //callBack:转换时的同步回调函数
    //  每次packet处理都会调用，若返回false则整个toPcm无效返回false
    //  回调函数<参数1>为输出缓冲区地址，<参数2>为输出数据有效字节长度
    //  （TODO 数据的左右声道也可以在这里区分）
    //返回实际读取数据的总长度，如果读取失败返回-1
    virtual qint64 readAll(std::function<bool (const char *, qint64)> callBack = nullptr) = 0;
    //读取所有数据
    //数据无效或已读取完则返回空
    //QByteArray readAll();
    //结束解码
    virtual void close() = 0;

    //是否open，正在转码
    bool isOpen() const;
    //是否数据转码完毕
    bool atEnd() const;

protected:
    void setOpen(bool open);
    void setEnd(bool end);

private:
    //open状态
    bool theOpened{ false };
    //转码结束
    bool theEnded{ true };

    //Q_DISABLE_COPY_MOVE(EasyAbstractDecoder)
};

class EASYAUDIOTOOL_EXPORT EasyAbstractEncoder
{
public:
private:
    //Q_DISABLE_COPY_MOVE(EasyAbstractEncoder)
};
