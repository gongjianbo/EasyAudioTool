#include "EasyAudioInterface.h"

#include <QDebug>

EasyAbstractContext::EasyAbstractContext(EasyAudio::CodecType type)
    : theType(type)
{

}

bool EasyAbstractContext::isValid() const
{
    return theValid;
}

EasyAudio::CodecType EasyAbstractContext::codecType() const
{
    return theType;
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

bool EasyAbstractDecoder::isOpen() const
{
    return theOpened;
}

bool EasyAbstractDecoder::isEnd() const
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
