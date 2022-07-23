#include "EasyAudioTool.h"
#include "EasyAudioFactory.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <QDebug>
#include <functional>

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

bool EasyAudioTool::transcodeToWavFile(const QString &srcPath, const QString &tagretPath,
                                       const QAudioFormat &tagretFormat, const std::atomic_bool &runflag)
{
    QFileInfo src_info(srcPath);
    QFileInfo dst_info(tagretPath);
    //原文件不存在
    //目标目录不存在或者未生成
    if(!src_info.exists() || !(dst_info.dir().exists() || dst_info.dir().mkpath(dst_info.absolutePath()))){
        qDebug()<<"transcode path error."<<src_info<<dst_info;
        return false;
    }

    //解码失败
    QSharedPointer<EasyAbstractDecoder> p_decoder = EasyAudioFactory::createDecoder(srcPath);
    if(!p_decoder || !p_decoder->isValid() || !p_decoder->open(tagretFormat)){
        qDebug()<<"transcode parse error. ";
        return false;
    }

    //存转换结果
    bool trans_ok = false;

    //缓冲区
    static const qint64 buffer_max = 1024*1024*10;
    QByteArray buffer;
    buffer.resize(buffer_max);
    //缓冲区已有数据大小
    qint64 buffer_has = 0;
    //写入目标文件
    QFile write_file(tagretPath);
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
    //writeonly时默认Truncate
    if(write_file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        EasyWavHead head;
        //头占位
        write_file.write(QByteArray((char*)&head, sizeof(EasyWavHead)));
        qint64 out_count = p_decoder->readAll(call_back);
        //没转出数据说明失败了
        trans_ok = (out_count>0 && runflag);
        if(trans_ok){
            //尾巴上那点写文件
            if(buffer_has>0){
                write_file.write(buffer.data(), buffer_has);
                buffer_has = 0;
            }
            //头覆盖
            write_file.seek(0);
            head = EasyWavHead(tagretFormat, out_count);
            write_file.write((const char*)&head, sizeof(EasyWavHead));
            trans_ok = head.isValid();
            if(!trans_ok){
                qDebug()<<"transcode error. head invalid.";
            }
        }else{
            qDebug()<<"transcode error. count"<<out_count<<"run flag"<<bool(runflag);
        }
        //关闭文件
        write_file.waitForBytesWritten(3000);
        write_file.close();
    }
    //解码结束
    p_decoder->close();

    //无效的转换就把那个文件删除
    if(!trans_ok){
        qDebug()<<"transcode write error. remove file.";
        write_file.remove();
    }

    return trans_ok;
}

auto EasyAudioTool::transcodeToWavFile(const QString &sourcePath, const QString &targetDir,
                                       const QAudioFormat &targetFormat, const qint64 &limitSize,
                                       const std::atomic_bool &runflag)
->QList<QPair<QString, QString>>
{
    //存转码结果信息
    QList<QPair<QString, QString>> trans_result;

    QFileInfo src_info(sourcePath);
    QDir tgt_dir(targetDir);
    //原文件不存在
    //目标目录不存在或者未生成
    if(!src_info.exists() || !(tgt_dir.exists() || tgt_dir.mkpath(targetDir))){
        qDebug()<<"transcode path error."<<src_info<<tgt_dir;
        return trans_result;
    }

    //解码失败
    QSharedPointer<EasyAbstractDecoder> p_decoder = EasyAudioFactory::createDecoder(sourcePath);
    if(!p_decoder || !p_decoder->isValid() || !p_decoder->open(targetFormat)){
        qDebug()<<"transcode parse error. ";
        return trans_result;
    }

    //缓冲区
    const qint64 buffer_max = 1024*1024*10;
    QByteArray buffer;
    buffer.resize(buffer_max);
    //缓冲区已有数据大小
    qint64 buffer_has = 0;
    //当前分片文件写出的大小
    qint64 slice_size = 0;
    //写入目标文件
    QString target_uuid; // = QUuid::createUuid().toString();
    QString target_path; // = QString("%1/%2.wav").arg(targetDir).arg(target_uuid);
    QFile write_file;

    //写文件操作
    std::function<bool(const char*, qint64)> call_write = [&](const char* pcmData, qint64 pcmSize)
    {
        qint64 write_offset = 0;
        qint64 pcm_has = pcmSize;
        while(pcm_has > 0){
            //每次写入前判断是否要新建文件
            if(!write_file.isOpen()){
                target_uuid = QUuid::createUuid().toString();
                target_path = QString("%1/%2.wav").arg(targetDir).arg(target_uuid);
                write_file.setFileName(target_path);
                if(!write_file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
                    return false;
                }
                EasyWavHead head;
                //头占位
                write_file.write(QByteArray((char*)&head, sizeof(EasyWavHead)));
                slice_size = 0;
                trans_result.push_back({target_uuid, target_path});
            }
            //qDebug()<<write_offset<<slice_size<<pcm_has<<limitSize;
            if(slice_size + pcm_has >= limitSize){
                write_file.write(pcmData+write_offset, (limitSize-slice_size));
                write_offset += (limitSize-slice_size);
                pcm_has -= (limitSize-slice_size);
                slice_size = limitSize;
                //头覆盖
                write_file.seek(0);
                EasyWavHead head = EasyWavHead(targetFormat, slice_size);
                write_file.write(QByteArray((char*)&head, sizeof(EasyWavHead)));
                write_file.close();
            }else{
                write_file.write(pcmData+write_offset, pcm_has);
                write_offset += pcm_has;
                slice_size += pcm_has;
                //全部写出，剩余长度置零
                pcm_has = 0;
            }
        }
        return true;
    };

    //转码实时写入目标文件
    //toPcm的回调参数
    std::function<bool(const char*, qint64)> call_back = [&](const char* pcmData, qint64 pcmSize)
    {
        //中断
        if(!runflag){
            return false;
        }
        if(pcmSize + buffer_has >= buffer_max){
            //先把缓存清空
            if(buffer_has > 0){
                if(!call_write(buffer.data(), buffer_has)){
                    return false;
                }
                buffer_has = 0;
            }
            if(pcmSize >= buffer_max){
                if(!call_write(pcmData, pcmSize)){
                    return false;
                }
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


    qint64 out_count = p_decoder->readAll(call_back);
    //没转出数据说明失败了
    const bool trans_ok = (out_count>0 && runflag);
    if(trans_ok){
        //尾巴上那点写文件
        if(buffer_has > 0){
            call_write(buffer.data(), buffer_has);
            buffer_has = 0;
        }
        //文件尾
        if(write_file.isOpen()){
            //头覆盖
            write_file.seek(0);
            EasyWavHead head = EasyWavHead(targetFormat, slice_size);
            write_file.write(QByteArray((char*)&head, sizeof(EasyWavHead)));
            write_file.close();
        }
    }else{
        write_file.close();
        qDebug()<<"transcode error. count"<<out_count<<"run flag"<<bool(runflag);
    }

    //解码结束
    p_decoder->close();

    //无效的转换就把文件删除
    if(!trans_ok){
        qDebug()<<"transcode write error. remove file.";
        //write_file.remove();
        for(const auto &trans_pair : qAsConst(trans_result))
        {
            QFile::remove(trans_pair.second);
        }
        trans_result.clear();
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
