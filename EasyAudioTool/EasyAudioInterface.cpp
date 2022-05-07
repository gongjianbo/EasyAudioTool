#include "EasyAudioInterface.h"

#include <QDebug>

EasyAbstractContext::EasyAbstractContext(EasyAudio::CodecType codecType)
    : theCodecType(codecType)
{

}

bool EasyAbstractContext::isValid() const
{
    return theValid;
}

EasyAudio::CodecType EasyAbstractContext::codecType() const
{
    return theCodecType;
}

QString EasyAbstractContext::lastError() const
{
    return theError;
}

void EasyAbstractContext::setValid(bool valid)
{
    theValid = valid;
}

void EasyAbstractContext::setLastError(const QString &info)
{
    theError = info;
    qDebug()<<"easy audio context error:"<<info;
}

bool EasyAbstractDecoder::reset()
{
    return seek(0);
}

QByteArray EasyAbstractDecoder::read(qint64 maxSize)
{
    QByteArray result;
    if(!isOpen() || atEnd() || maxSize<1){
        return result;
    }
    result.resize(maxSize);
    const qint64 read_size = read(result.data(),maxSize);
    if(read_size<1){
        result.clear();
    }else if(read_size<maxSize){
        result = result.left(read_size);
    }
    return result;
}

bool EasyAbstractDecoder::isOpen() const
{
    return theOpened;
}

bool EasyAbstractDecoder::atEnd() const
{
    return theEnded;
}

void EasyAbstractDecoder::setOpen(bool open)
{
    theOpened = open;
}

void EasyAbstractDecoder::setEnd(bool end)
{
    theEnded = end;
}
