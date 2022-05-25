#include "EasyAudioTool.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <functional>

#include "EasyAudioFactory.h"

void EasyAudioTool::registerQmlType(QQmlEngine *engine)
{
    QQmlContext *context = engine->rootContext();context;
    qmlRegisterType<EasyAudio>("EasyAudioTool",1,0,"EasyAudio");
    qmlRegisterType<EasyAudioPlayer>("EasyAudioTool",1,0,"EasyAudioPlayer");
}

void EasyAudioTool::registerMetaType()
{
    qRegisterMetaType<EasyWavHead>("EasyWavHead");
    qRegisterMetaType<EasyAudioInfo>("EasyAudioInfo");
}

bool EasyAudioTool::transcodeToWavFile(const QString &srcPath, const QString &destPath,
                                       const QAudioFormat &destFormat, const std::atomic_bool &runflag)
{
    QFileInfo src_info(srcPath);
    QFileInfo dst_info(destPath);
    //原文件不存在
    //目标目录不存在或者未生成
    if(!src_info.exists() || !(dst_info.dir().exists() || dst_info.dir().mkpath(dst_info.absolutePath()))){
        qDebug()<<"transcode path error."<<src_info<<dst_info;
        return false;
    }

    //解码失败
    QSharedPointer<EasyAbstractDecoder> p_decoder = EasyAudioFactory::createDecoder(srcPath);
    if(!p_decoder || !p_decoder->isValid() || !p_decoder->open(destFormat)){
        qDebug()<<"transcode parse error. ";
        return false;
    }

    //存转换结果
    bool trans_result = false;

    //缓冲区
    static const qint64 buffer_max = 1024*1024*10;
    QByteArray buffer;
    buffer.resize(buffer_max);
    //缓冲区已有数据大小
    qint64 buffer_has = 0;
    //写入目标文件
    QFile write_file(destPath);
    //writeonly时默认Truncate
    if(write_file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        EasyWavHead head;
        //头占位
        write_file.write(QByteArray((char*)&head, sizeof(EasyWavHead)));
        //转码实时写入目标文件
        //toPcm的回调参数
        std::function<bool(const char*, qint64)> call_back = [&](const char* pcmData, qint64 pcmSize)
        {
            //中断
            if(!runflag){
                return false;
            }
            if(pcmSize + buffer_has >= buffer_max){
                if(buffer_has > 0){
                    write_file.write(buffer.data(), buffer_has);
                    buffer_has = 0;
                }
                if(pcmSize >= buffer_max){
                    write_file.write(pcmData, pcmSize);
                }else{
                    memcpy(buffer.data()+buffer_has, pcmData, pcmSize);
                    buffer_has += pcmSize;
                }
            }else{
                memcpy(buffer.data()+buffer_has, pcmData, pcmSize);
                buffer_has += pcmSize;
            }
            return true;
        };
        qint64 dest_count = p_decoder->readAll(call_back);
        //没转出数据说明失败了
        trans_result = (dest_count>0 && runflag);
        if(trans_result){
            //尾巴上那点写文件
            if(buffer_has>0){
                write_file.write(buffer.data(), buffer_has);
                buffer_has = 0;
            }
            //头覆盖
            write_file.seek(0);
            head = EasyWavHead(destFormat, dest_count);
            write_file.write((const char*)&head, sizeof(EasyWavHead));
            trans_result = head.isValid();
            if(!trans_result){
                qDebug()<<"transcode error. head invalid.";
            }
        }else{
            qDebug()<<"transcode error. count"<<dest_count<<"run flag"<<bool(runflag);
        }
        //关闭文件
        write_file.waitForBytesWritten(3000);
        write_file.close();
    }
    //解码结束
    p_decoder->close();

    //无效的转换就把那个文件删除
    if(!trans_result){
        qDebug()<<"transcode write error. remove file.";
        write_file.remove();
    }

    return trans_result;
}

QAudioFormat EasyAudioTool::defaultFormat()
{
    QAudioFormat format;
    format.setChannelCount(1);
    format.setSampleRate(16000);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setCodec("audio/pcm");
    return format;
}
