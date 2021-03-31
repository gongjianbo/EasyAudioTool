#pragma once
//本文件用于放置接口定义
#include <QObject>
#include "EasyAudioDefine.h"

/**
 * @brief 编解码上下文基类
 * @author 龚建波
 * @date 2021-03-30
 */
class EasyAbstractContext
{
public:
    explicit EasyAbstractContext(EasyAudio::CodecType type);
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
    EasyAudio::CodecType theType;
    //错误信息，不同解码器状态码有区别，所以用字符串保存错误信息
    QString theError;

    Q_DISABLE_COPY_MOVE(EasyAbstractContext)
};

/**
 * @brief 解码器基类
 * @author 龚建波
 * @date 2021-03-30
 */
class EasyAbstractDecoder
{
public:
    EasyAbstractDecoder() = default;
    virtual ~EasyAbstractDecoder() = default;

    //绑定一个上下文进行处理
    //virtual void setContext(const QSharedPointer<EasyAbstractContext> &contextPtr) = 0;
    //是否为有效的音频文件（该解码器可解析）
    //判断为有效时才进行后续操作
    virtual bool isValid() const = 0;
    //开始解码
    //format:转出参数
    //return false:表示无法转换
    virtual bool open(const QAudioFormat &format) = 0;
    //设置读取位置，暂时忽略
    //virtual bool seek(qint64 offset) = 0;
    //转码所有数据
    //转码失败则返回数据为空
    virtual QByteArray readAll() = 0;
    //转码数据
    //size:期望返回的数据的最大长度
    //转码失败或者结束则返回数据为空
    virtual QByteArray read(qint64 maxSize) = 0;
    //转码数据，暂时忽略
    //virtual qint64 read(char *outBuffer, qint64 maxSize) = 0;
    //结束解码
    virtual void close() = 0;

    //是否open，正在转码
    bool isOpen() const;
    //是否数据转码完毕
    bool isEnd() const;

protected:
    void setOpen(bool open);
    void setEnd(bool end);

private:
    //open状态
    bool theOpened = false;
    //转码结束
    bool theEnded = true;

    //Q_DISABLE_COPY_MOVE(EasyAbstractDecoder)
};

class EasyAbstractEncoder
{
public:
private:
    //Q_DISABLE_COPY_MOVE(EasyAbstractEncoder)
};
