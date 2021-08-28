#pragma once
#include <QIODevice>

/**
 * @brief 音频数据缓冲区
 * @author 龚建波
 * @date 2021-03-31
 * @details
 * 配合QAudioOutput等进行播放
 * QAudioOutput需要QIODevice作为数据源
 */
class EasyPlayerBuffer : public QIODevice
{
    Q_OBJECT
public:
    explicit EasyPlayerBuffer(QObject *parent = nullptr);

    //需函数接口用于数据读写回调
    //播放数据调用
    qint64 readData(char *data, qint64 maxSize) override;
    //输入数据调用
    qint64 writeData(const char *data, qint64 maxSize) override;

    //【】状态&&标志
    void resetBuffer();
    qint64 getReadCount() const { return readCount; }
    qint64 getWriteCount() const { return writeCount; }
    //未读取的数据量
    qint64 getUnreadCount() const { return writeCount-readCount; }
    //当前buffer存量不足getMaxLength的一半则建议写入
    bool isWaitWrite() const;

    //是否读取完毕，配合writeEnd
    bool isReadEnd() const { return (writeEnd && readCount>=writeCount); }

    //是否数据写入完毕（由外部设置）
    bool isWriteEnd() const { return writeEnd; }
    void setWriteEnd(bool flag);

    //设置完整数据
    void setData(const QByteArray &data);
    //填充数据流数据
    void appendData(const QByteArray &data);

    //append后保留的最大长度
    static constexpr qint64 getMaxLength(){
        return 100*1024*1024;
    }
    //一次建议的写入长度16000*2*1*60
    static constexpr qint64 getOnceLength(){
        return 16000*2*1*60;
    }

signals:
    //当writeEnd时，未读取数据为0则触发
    //（只表示读取了，还需要时间播放，测试时默认读一次16384，所以延迟1s左右stop）
    void readEnded();

private:
    //待播放的的数据
    QByteArray audioData;
    //data[0]偏移相对于writeCount的偏移
    //（appendData累积到一定大小会移除前面的数据，被移除的就作为偏移取值readCount-audioOffset）
    qint64 audioOffset{ 0 };
    //readData累计
    qint64 readCount{ 0 };
    //writeData累计
    qint64 writeCount{ 0 };

    //是否数据写入完毕（由外部设置）
    //(边解析边播放需要)
    bool writeEnd{ false };
    //readData读完后还会触发多次
    //增加一个标记避免重复触发
    bool readEnd{ false };
};
