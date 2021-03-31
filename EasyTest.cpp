#include "EasyTest.h"

#include "EasyAudioRegister.h"
#include <QFile>
#include <QDebug>
#include <thread>

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
    });
    th.detach();
}

void EasyTest::FFmpegRead(const QString &filepath)
{
    QSharedPointer<EasyFFmpegContext> cp(new EasyFFmpegContext(filepath));
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
    QSharedPointer<EasyFFmpegContext> cp(new EasyFFmpegContext(filepath));
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
    QSharedPointer<EasySilkContext> cp(new EasySilkContext(filepath));
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
    QSharedPointer<EasySilkContext> cp(new EasySilkContext(filepath));
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
