#include "EasyTest.h"

#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <thread>

#include "EasyFFmpegContext.h"
#include "EasyFFmpegDecoder.h"
#include "EasySilkContext.h"
#include "EasySilkDecoder.h"
#include "EasyAudioFactory.h"

QAudioFormat fmt;

void EasyTest::testDecoder()
{
    fmt.setCodec("audio/pcm");
    fmt.setChannelCount(1);
    fmt.setSampleRate(16000);
    fmt.setSampleSize(16);
    fmt.setSampleType(QAudioFormat::SignedInt);

    std::thread th([this]{
        const QString app_dir=qApp->applicationDirPath();

        const QString ffmpeg_src=app_dir+"/audio/audio.flac";
        tFFmpegRead(ffmpeg_src);
        tFFmpegReadAll(ffmpeg_src);
        tFFmpegCallBack(ffmpeg_src);

        const QString silk_src=app_dir+"/audio/weixin.amr";
        tSilkRead(silk_src);
        tSilkReadAll(silk_src);
        tSilkCallBack(silk_src);

        tFactoryContext(ffmpeg_src);
        tFactoryDecoder(silk_src);
        fmt.setChannelCount(2);
        tFactoryDecoder(ffmpeg_src);
    });
    th.detach();
}

void EasyTest::tFFmpegRead(const QString &filepath)
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

void EasyTest::tFFmpegReadAll(const QString &filepath)
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

void EasyTest::tFFmpegCallBack(const QString &filepath)
{
    QSharedPointer<EasyAbstractContext> cp(new EasyFFmpegContext(filepath));
    EasyFFmpegDecoder dec;
    dec.setContext(cp);
    bool result=false;
    if(dec.open(fmt)){
        //转pcm回调
        QByteArray pcm_temp;
        unsigned int size_count = 0;
        QFile write_file(filepath+"_ffmpegcallback.wav");
        std::function<bool(const char*,int)> call_back=[&](const char* pcmData,int pcmSize)
        {
            //每次只写一点速度比较慢
            //write_file.write(pcmData,pcmSize);
            pcm_temp.append(pcmData,pcmSize);
            size_count+=pcmSize;
            //每次写10M
            if(pcm_temp.count()>1024*1024*10){
                write_file.write(pcm_temp);
                pcm_temp.clear();
            }
            return true;
        };

        EasyWavHead head;
        if(write_file.open(QIODevice::WriteOnly)){
            //头占位
            write_file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
            //数据
            QByteArray test_temp=dec.readAll(call_back);
            if(!test_temp.isEmpty())
                qDebug()<<"readAll return not empty.";
            //没转出数据说明失败了
            result=(size_count>0);
            if(result){
                //尾巴上那点写文件
                write_file.write(pcm_temp);
                //头覆盖
                write_file.seek(0);
                head=EasyWavHead::createHead(fmt,size_count);
                write_file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
            }
            write_file.close();
        }
        //无效的就移除该文件
        if(!result){
            write_file.remove();
        }
        dec.close();
    }
    qDebug()<<"Test ffmpegcallback result"<<result;
}

void EasyTest::tSilkRead(const QString &filepath)
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

void EasyTest::tSilkReadAll(const QString &filepath)
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

void EasyTest::tSilkCallBack(const QString &filepath)
{
    QSharedPointer<EasyAbstractContext> cp(new EasySilkContext(filepath));
    EasySilkDecoder dec;
    dec.setContext(cp);
    bool result=false;
    if(dec.open(fmt)){
        //转pcm回调
        QByteArray pcm_temp;
        unsigned int size_count = 0;
        QFile write_file(filepath+"_silkcallback.wav");
        std::function<bool(const char*,int)> call_back=[&](const char* pcmData,int pcmSize)
        {
            //每次只写一点速度比较慢
            //write_file.write(pcmData,pcmSize);
            pcm_temp.append(pcmData,pcmSize);
            size_count+=pcmSize;
            //每次写10M
            if(pcm_temp.count()>1024*1024*10){
                write_file.write(pcm_temp);
                pcm_temp.clear();
            }
            return true;
        };

        EasyWavHead head;
        if(write_file.open(QIODevice::WriteOnly)){
            //头占位
            write_file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
            //数据
            QByteArray test_temp=dec.readAll(call_back);
            if(!test_temp.isEmpty())
                qDebug()<<"readAll return not empty.";
            //没转出数据说明失败了
            result=(size_count>0);
            if(result){
                //尾巴上那点写文件
                write_file.write(pcm_temp);
                //头覆盖
                write_file.seek(0);
                head=EasyWavHead::createHead(fmt,size_count);
                write_file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
            }
            write_file.close();
        }
        //无效的就移除该文件
        if(!result){
            write_file.remove();
        }
        dec.close();
    }
    qDebug()<<"Test silkcallback result"<<result;
}

void EasyTest::tFactoryContext(const QString &filepath)
{
    QSharedPointer<EasyAbstractContext> sp=EasyAudioFactory::createContext(filepath);
    bool result=false;
    if(sp&&sp->isValid()){
        EasyAudioInfo info = sp->audioInfo();
        result=info.valid;
    }
    qDebug()<<"Test factory context result"<<result;
}

void EasyTest::tFactoryDecoder(const QString &filepath)
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

void EasyTest::testPlayer()
{
    const QString app_dir=qApp->applicationDirPath();
    const QString ffmpeg_src=app_dir+"/audio/audio.mp3";
    player.play(ffmpeg_src);
}

