#include "EasyTest.h"

#include <QFile>
#include <QDebug>
#include <thread>

#include "EasyFFmpegContext.h"
#include "EasyFFmpegDecoder.h"
#include "EasySilkContext.h"
#include "EasySilkDecoder.h"
#include "EasyAudioFactory.h"

QAudioFormat fmt;

void EasyTest::TestAll()
{
    fmt.setCodec("audio/pcm");
    fmt.setChannelCount(1);
    fmt.setSampleRate(16000);
    fmt.setSampleSize(16);
    fmt.setSampleType(QAudioFormat::SignedInt);

    std::thread th([this]{
        const QString ffmpeg_src="D:/Temp/1.flac";
        FFmpegRead(ffmpeg_src);
        FFmpegReadAll(ffmpeg_src);

        const QString silk_src="D:/Temp/weixin.amr";
        SilkRead(silk_src);
        SilkReadAll(silk_src);

        FactoryContext(ffmpeg_src);
        FactoryDecoder(silk_src);
    });
    th.detach();
}

void EasyTest::FFmpegRead(const QString &filepath)
{
    QSharedPointer<EasyAbstractContext> cp(new EasyFFmpegContext(filepath));
    EasyFFmpegDecoder dec;
    dec.setContext(cp);
    bool result=false;
    if(dec.open(fmt)){
        QByteArray data;
        while(!dec.isEnd()){
            data+=dec.read(1024*1024);
        }
        //QByteArray data=dec.readAll();
        EasyWavHead head=EasyWavHead::createHead(fmt,data.size());
        QFile file(filepath+"_ffmpegread.wav");
        if(data.size()>0&&file.open(QIODevice::WriteOnly)){
            file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
            file.write(data);
            file.close();
            result=true;
        }
        dec.close();
    }
    qDebug()<<"Test ffmpegread result"<<result;
}

void EasyTest::FFmpegReadAll(const QString &filepath)
{
    QSharedPointer<EasyAbstractContext> cp(new EasyFFmpegContext(filepath));
    EasyFFmpegDecoder dec;
    dec.setContext(cp);
    bool result=false;
    if(dec.open(fmt)){
        QByteArray data=dec.readAll();
        EasyWavHead head=EasyWavHead::createHead(fmt,data.size());
        QFile file(filepath+"_ffmpegreadall.wav");
        if(data.size()>0&&file.open(QIODevice::WriteOnly)){
            file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
            file.write(data);
            file.close();
            result=true;
        }
        dec.close();
    }
    qDebug()<<"Test ffmpegreadall result"<<result;
}

void EasyTest::SilkRead(const QString &filepath)
{
    QSharedPointer<EasyAbstractContext> cp(new EasySilkContext(filepath));
    EasySilkDecoder dec;
    dec.setContext(cp);
    bool result=false;
    if(dec.open(fmt)){
        QByteArray data;
        while(!dec.isEnd()){
            data+=dec.read(1024*1024);
        }
        //QByteArray data=dec.readAll();
        EasyWavHead head=EasyWavHead::createHead(fmt,data.size());
        QFile file(filepath+"_silkread.wav");
        if(data.size()>0&&file.open(QIODevice::WriteOnly)){
            file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
            file.write(data);
            file.close();
            result=true;
        }
        dec.close();
    }
    qDebug()<<"Test silkread result"<<result;
}

void EasyTest::SilkReadAll(const QString &filepath)
{
    QSharedPointer<EasyAbstractContext> cp(new EasySilkContext(filepath));
    EasySilkDecoder dec;
    dec.setContext(cp);
    bool result=false;
    if(dec.open(fmt)){
        QByteArray data=dec.readAll();
        EasyWavHead head=EasyWavHead::createHead(fmt,data.size());
        QFile file(filepath+"_silkreadall.wav");
        if(data.size()>0&&file.open(QIODevice::WriteOnly)){
            file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
            file.write(data);
            file.close();
            result=true;
        }
        dec.close();
    }
    qDebug()<<"Test silkreadall result"<<result;
}

void EasyTest::FactoryContext(const QString &filepath)
{
    QSharedPointer<EasyAbstractContext> sp=EasyAudioFactory::createContext(filepath);
    bool result=false;
    if(sp&&sp->isValid()){
        EasyAudioInfo info = sp->audioInfo();
        result=info.valid;
    }
    qDebug()<<"Test factory context result"<<result;
}

void EasyTest::FactoryDecoder(const QString &filepath)
{
    QSharedPointer<EasyAbstractDecoder> sp=EasyAudioFactory::createDecoder(filepath);
    bool result=false;
    if(sp&&sp->isValid()&&sp->open(fmt)){
        QByteArray data=sp->readAll();
        EasyWavHead head=EasyWavHead::createHead(fmt,data.size());
        QFile file(filepath+"_factorydecoder.wav");
        if(data.size()>0&&file.open(QIODevice::WriteOnly)){
            file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
            file.write(data);
            file.close();
            result=true;
        }
        sp->close();
    }
    qDebug()<<"Test factory decoder result"<<result;
}
