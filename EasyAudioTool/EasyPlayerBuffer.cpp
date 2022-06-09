#include "EasyPlayerBuffer.h"

#include <QTimer>
#include <QDebug>

EasyPlayerBuffer::EasyPlayerBuffer(QObject *parent)
    : QIODevice(parent)
{

}

qint64 EasyPlayerBuffer::readData(char *data, qint64 maxSize)
{
    if(!data || maxSize<1)
        return 0;
    const int data_size = getUnreadCount();
    if(data_size <= 0){
        //如果读取完了就发送信号
        //判断readEnd避免重复触发
        if(writeEnd && !readEnd){
            readEnd = true;
            emit readEnded();
        }
        return 0;
    }
    readEnd = false;
    //本次读取大小
    const int read_size = (data_size>=maxSize) ? maxSize : data_size;
    //readCount和audioOffset都包含了从data移除的部分，
    //相减之后就去掉了data移除的部分，可以用作偏移
    int read_offset = readCount-audioOffset;
    if(read_offset < 0){
        qDebug()<<"readData error, read offset < 0";
        return 0;
    }
    memcpy(data,audioData.constData()+read_offset,read_size);
    readCount += read_size;
    return read_size;
}

qint64 EasyPlayerBuffer::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    //目前只有播放没有录制，略
    return -1;
}

void EasyPlayerBuffer::resetBuffer()
{
    reset();
    readCount = 0;
    writeCount = 0;
    audioOffset = 0;
    audioData.clear();
    readEnd = false;
    writeEnd = false;
}

bool EasyPlayerBuffer::isWaitWrite() const
{
    //当前缓冲区数据量小于容量的一半
    return (audioData.count() < (getMaxLength()/2));
}

void EasyPlayerBuffer::setWriteEnd(bool flag)
{
    //外部写入数据结束后设置该标志
    if(writeEnd != flag){
        writeEnd = flag;
    }
}

void EasyPlayerBuffer::setData(const QByteArray &data)
{
    //设置完整的播放数据，区别于appendData
    audioData = data;
    writeCount = data.size();
    writeEnd = true;
}

void EasyPlayerBuffer::appendData(const QByteArray &data)
{
    //追加到末尾
    audioData.append(data);
    //已写入总量
    writeCount += data.size();

    //最大只放xx M，如果超过容量限制就移除一部分
    if(audioData.count() > getMaxLength()){
        //offset表示待移除的长度
        qint64 offset = 0;
        //有已读的就移除已读的，没有就丢弃一部分未读的
        if(getUnreadCount() < getMaxLength()){
            //count-未读=已读
            offset = audioData.count()-getUnreadCount();
        }else{
            //全是未读的就丢弃一部分
            offset = getMaxLength()/2;
            //丢弃的部分当作已读取
            readCount += offset;
        }

        if(offset > 0){
            //readCount-audioOffset=读取时audioData的偏移
            audioOffset += offset;
            //移除这一部分
            audioData.remove(0,offset);
        }
    }
}
