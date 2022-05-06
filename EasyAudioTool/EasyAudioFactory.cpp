#include "EasyAudioFactory.h"

#include "EasyFFmpegContext.h"
#include "EasyFFmpegDecoder.h"
#include "EasySilkContext.h"
#include "EasySilkDecoder.h"

QSharedPointer<EasyAbstractContext>
EasyAudioFactory::createContext(const QString &filepath)
{
    if(EasySilkContext::isSilkAudio(filepath)){
        return QSharedPointer<EasyAbstractContext>(new EasySilkContext(filepath));
    }
    return QSharedPointer<EasyAbstractContext>(new EasyFFmpegContext(filepath));
}

QSharedPointer<EasyAbstractDecoder>
EasyAudioFactory::createDecoder(const QString &filepath)
{
    QSharedPointer<EasyAbstractContext> context = createContext(filepath);
    EasyAbstractDecoder *dec;
    switch(context->codecType())
    {
    case EasyAudio::SilkV3:
        dec = new EasySilkDecoder;
        break;
        //case EasyAudio::FFmpeg: break;
    default:
        dec = new EasyFFmpegDecoder;
        break;
    }
    dec->setContext(context);
    return QSharedPointer<EasyAbstractDecoder>(dec);
}
