#include "EasyAudioTool.h"

#include "EasyAudioFactory.h"
#include <QCoreApplication>
#include <QtConcurrentRun>
#include <QFileInfo>
#include <QFuture>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <QDebug>

EasyAudioTool::EasyAudioTool(QObject *parent) : QObject(parent)
{
    setCacheDir(qApp->applicationDirPath()+"/easyCache");
}

EasyAudioTool::~EasyAudioTool()
{
    if(!taskWatcher.isFinished()){
        stop();
        taskWatcher.waitForFinished();
    }
}

void EasyAudioTool::setSupportedSuffixs(const QStringList &filter)
{
    supportedSuffixs = filter;
    emit supportedSuffixsChanged();
}

void EasyAudioTool::setCacheDir(const QString &dir)
{
    if(cacheDir != dir){
        cacheDir = dir;
        emit cacheDirChanged(cacheDir);
    }
}

QList<QString> EasyAudioTool::urlsToStrs(const QList<QUrl> &urls)
{
    QList<QString> file_list;
    for(const QUrl &dir : urls)
    {
        file_list.push_back(dir.toLocalFile());
    }
    return file_list;
}

void EasyAudioTool::setProcessing(bool on, int count)
{
    if(onProcess != on){
        onProcess = on;
        emit processStatusChanged();
        if(on){
            emit processStarted(count);
            qDebug()<<QString("process start. count %1.")
                      .arg(count);
        }else{
            emit processFinished(bool(count>0));
            qDebug()<<QString("process finish. result %1. count %2 success %3 fail %4")
                      .arg(bool(count>0))
                      .arg(processCount)
                      .arg(processSuccess)
                      .arg(processFail);
        }
    }
}

void EasyAudioTool::setProcessProgress(int count, int success, int fail)
{
    processCount = count;
    processSuccess = success;
    processFail = fail;
    emit processProgressChanged(processCount,processSuccess,processFail);
}

void EasyAudioTool::updateProcessProgress(bool result)
{
    if(result){
        processSuccess += 1;
    }else{
        processFail += 1;
    }
    emit processProgressChanged(processCount,processSuccess,processFail);
}

void EasyAudioTool::parsePathList(const QList<QString> &files,
                                  bool useFilter,
                                  const QStringList &filter)
{
    if(files.isEmpty() || !taskWatcher.isFinished()){
        qDebug()<<"tool parsePathList start failed.";
        return;
    }

    setProcessing(true,files.count());
    setProcessProgress(files.count(),0,0);
    //因为解析的速度比较快，所以暂时不emit进度值，除非文件数较多
    QFuture<void> future = QtConcurrent::run(&taskPool,[=]{
        //解析失败的列表
        QStringList failed_list;
        QStringList filter_list;
        if(useFilter){
            filter_list = filter.isEmpty()?getSupportedSuffixs():filter;
        }
        //计数
        int file_count = files.count();
        int success_count = 0;
        int fail_count = 0;

        for(int index=0; index<files.count(); index++)
        {
            if(!getProcessing()){
                break;
            }
            fail_count++;
            const QString &filepath = files.at(index);

            //非文件、或者文件不存在不放到失败列表
            if(!QFileInfo(filepath).isFile()){
                continue;
            }
            //文件后缀校验
            if(useFilter){
                const QString filename = QFileInfo(filepath).fileName();
                //过滤掉的后缀不处理
                if(!QDir::match(filter_list,filename)){
                    failed_list.push_back(filepath);
                    continue;
                }
            }
            //提取音频信息
            QSharedPointer<EasyAbstractContext> p_context = EasyAudioFactory::createContext(filepath);
            if(p_context && p_context->isValid()){
                EasyAudioInfo info = p_context->audioInfo();
                if(info.valid){
                    emit parseFinished(info);
                    fail_count--;
                    success_count++;
                    continue;
                }
            }

            failed_list.push_back(filepath);
        }

        setProcessProgress(file_count,success_count,fail_count);
        if(getProcessing()){
            if(!failed_list.isEmpty())
                emit parseFailedChanged(failed_list);
        }
        //process=false表示人为终止
        setProcessing(false,getProcessing()?1:-1);
    });
    //connect(taskWatcher,&QFutureWatcher<void>::finished,this,[this]{});
    //在调用stFuture之前应建立connection以避免争用,Future可能在connection完成之前完成。
    taskWatcher.setFuture(future);
}

void EasyAudioTool::parseUrlList(const QList<QUrl> &files,
                                 bool useFilter,
                                 const QStringList &filter)
{
    parsePathList(urlsToStrs(files),useFilter,filter);
}

void EasyAudioTool::parseDirPath(const QDir &dir,
                                 bool useFilter,
                                 const QStringList &filter)
{
    if(dir.exists()){
        //如果过滤器为空，则使用默认的过滤列表
        QStringList filter_list;
        if(useFilter){
            filter_list = filter.isEmpty()?getSupportedSuffixs():filter;
        }
        const QFileInfoList info_list = dir.entryInfoList(filter_list,QDir::Files|QDir::NoSymLinks);

        QList<QString> file_list;
        for(const QFileInfo &info : info_list)
            file_list.push_back(info.filePath());
        parsePathList(file_list,false);
    }else{
        qDebug()<<"tool parseDirPath dir not exists.";
    }
}

void EasyAudioTool::parseDirUrl(const QUrl &dir,
                                bool useFilter,
                                const QStringList &filter)
{
    //QtQuick.Dialogs使用的url存储路径
    //url转换为目录
    //qDebug()<<"parse dir"<<dir.toLocalFile();
    parseDirPath(QDir(dir.toLocalFile()),useFilter,filter);
}

void EasyAudioTool::transcodePathList(const QList<QString> &files, const QString &dstdir)
{
    if(files.isEmpty() || !taskWatcher.isFinished()){
        qDebug()<<"tool transcodePathList start failed.";
        return;
    }

    setProcessing(true,files.count());
    setProcessProgress(files.count(),0,0);

    //待使用的变量
    QAudioFormat format = targetFormat;
    const QString cache_dir = dstdir.isEmpty()?getCacheDir():dstdir;

    QFuture<void> future = QtConcurrent::run(&taskPool,[=]{
        //解析失败的列表
        QStringList failed_list;
        //计数
        int file_count = files.count();
        int success_count = 0;
        int fail_count = 0;

        for(int index=0; index<files.count(); index++)
        {
            if(!getProcessing()){
                break;
            }
            fail_count++;
            const QString &filepath = files.at(index);
            //转换后的文件以uuid命名
            const QString uuid = QUuid::createUuid().toString();
            const QString dstpath = QString("%1/%2.%3")
                    .arg(cache_dir).arg(uuid).arg(format.codec());
            const EasyAudioInfo info = transcodeFile(filepath,dstpath,format);
            //转换成功的就emit除去
            if(info.valid){
                transcodeFinished(info);
                fail_count--;
                success_count++;
            }else{
                failed_list.push_back(filepath);
            }
            //目前没有break、continue等跳过，只放一处即可
            setProcessProgress(file_count,success_count,fail_count);
        }

        //setProcessProgress(file_count,success_count,fail_count);
        if(getProcessing()){
            if(!failed_list.isEmpty())
                emit parseFailedChanged(failed_list);
        }
        //process=false表示人为终止
        setProcessing(false,getProcessing()?1:-1);
    });
    taskWatcher.setFuture(future);
}

void EasyAudioTool::transcodeUrlList(const QList<QUrl> &files, const QString &dstdir)
{
    transcodePathList(urlsToStrs(files),dstdir);
}

EasyAudioInfo EasyAudioTool::transcodeFile(const QString &srcpath,
                                           const QString &dstpath,
                                           const QAudioFormat &format)
{
    EasyAudioInfo info;
    info.filepath = dstpath;
    info.valid = false;

    //TODO 目前encode类未完成，所以只能转为wav-pcm格式的文件
    if(format.codec() != "wav")
        return info;

    QFileInfo src_info(srcpath);
    QFileInfo dst_info(dstpath);
    //原文件不存在
    //目标目录不存在或者未生成
    if(!src_info.exists() || !(dst_info.dir().exists() || dst_info.dir().mkpath(dst_info.absolutePath()))){
        qDebug()<<"transcode path error."<<src_info<<dst_info;
        return info;
    }

    //解码开始
    QSharedPointer<EasyAbstractDecoder> p_decoder = EasyAudioFactory::createDecoder(srcpath);
    if(!p_decoder || !p_decoder->isValid() || !p_decoder->open(format)){
        qDebug()<<"transcode parse error. ";
        return info;
    }
    //存转换结果
    bool trans_result = false;
    QFile write_file(dstpath);
    //转pcm回调
    QByteArray pcm_temp;
    unsigned int size_count = 0;
    std::function<bool(const char*,int)> call_back = [&](const char* pcmData,int pcmSize)
    {
        //每次只写一点速度比较慢
        pcm_temp.append(pcmData,pcmSize);
        size_count += pcmSize;
        //每次写10M
        if(pcm_temp.count() > 1024*1024*10){
            write_file.write(pcm_temp);
            pcm_temp.clear();
        }
        return true;
    };
    //writeonly时默认Truncate
    if(write_file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        EasyWavHead head;
        //头占位
        write_file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
        //数据
        QByteArray test_temp = p_decoder->readAll(call_back);
        if(!test_temp.isEmpty())
            qDebug()<<"transcode readAll return not empty.";
        //没转出数据说明失败了
        trans_result = (size_count>0);
        if(trans_result){
            //尾巴上那点写文件
            write_file.write(pcm_temp);
            //头覆盖
            write_file.seek(0);
            head = EasyWavHead::createHead(format,size_count);
            write_file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
            //关闭，以便context打开
            write_file.close();
            //信息的文件信息
            QSharedPointer<EasyAbstractContext> p_context = EasyAudioFactory::createContext(dstpath);
            if(p_context && p_context->isValid()){
                info = p_context->audioInfo();
                trans_result = info.valid;
            }
        }else{
            write_file.close();
        }
    }
    //解码结束
    p_decoder->close();

    //无效的转换就把那个文件删除
    if(!trans_result){
        qDebug()<<"transcode write error. remove file.";
        write_file.remove();
    }
    info.valid = trans_result;

    return info;
}

void EasyAudioTool::stitchPathList(const QList<QString> &files, const QString &dstpath)
{
    if(files.isEmpty() || !taskWatcher.isFinished()){
        qDebug()<<"tool stitchPathList start failed.";
        return;
    }

    setProcessing(true,files.count());
    setProcessProgress(files.count(),0,0);

    //待使用的变量
    QAudioFormat format = targetFormat;
    const QString outpath = dstpath.isEmpty()
            ?QString("%1/stitch.%2").arg(getCacheDir()).arg(format.codec())
           :dstpath;

    QFuture<void> future = QtConcurrent::run(&taskPool,[=]{
        EasyAudioInfo info;
        info.filepath = outpath;
        info.valid = false;
        QFileInfo outinfo(outpath);
        //原文件不存在
        //目标目录不存在或者未生成
        if(!(outinfo.dir().exists() || outinfo.dir().mkpath(outinfo.absolutePath()))){
            qDebug()<<"stitch path error."<<outpath;
            emit stitchFinished(info);
            setProcessing(false,-1);
            return;
        }
        //拼接结果
        bool stitch_result = false;
        QFile write_file(outpath);
        //转pcm回调
        QByteArray pcm_temp;
        unsigned int size_count = 0;
        std::function<bool(const char*,int)> call_back = [&](const char* pcmData,int pcmSize)
        {
            //每次只写一点速度比较慢
            pcm_temp.append(pcmData,pcmSize);
            size_count += pcmSize;
            //每次写10M
            if(pcm_temp.count() > 1024*1024*10){
                write_file.write(pcm_temp);
                pcm_temp.clear();
            }
            return true;
        };
        //writeonly时默认Truncate
        if(write_file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
            //计数
            int file_count = files.count();
            int success_count = 0;
            int fail_count = 0;
            EasyWavHead head;
            //头占位
            write_file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));

            stitch_result = true;
            for(int index=0; index<files.count(); index++)
            {
                //stitch_result放这里break，就只写一个progress
                if(!getProcessing() || !stitch_result){
                    stitch_result = false;
                    break;
                }
                const QString &filepath = files.at(index);
                QSharedPointer<EasyAbstractDecoder> p_decoder = EasyAudioFactory::createDecoder(filepath);
                if(!p_decoder || !p_decoder->isValid() || !p_decoder->open(format)){
                    qDebug()<<"stitch parse error. ";
                    stitch_result = false;
                }else{
                    p_decoder->readAll(call_back);
                    p_decoder->close();
                }

                //转换成功的就emit除去
                if(stitch_result){
                    success_count++;
                }else{
                    //任一个失败则整体失败
                    fail_count++;
                }
                //目前没有break、continue等跳过，只放一处即可
                setProcessProgress(file_count,success_count,fail_count);
            }

            //没转出数据说明失败了
            if(size_count<1)
                stitch_result = false;
            if(stitch_result){
                //尾巴上那点写文件
                write_file.write(pcm_temp);
                //头覆盖
                write_file.seek(0);
                head = EasyWavHead::createHead(format,size_count);
                write_file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
                write_file.close();
                //信息的文件信息
                QSharedPointer<EasyAbstractContext> p_context = EasyAudioFactory::createContext(dstpath);
                if(p_context && p_context->isValid()){
                    info = p_context->audioInfo();
                    stitch_result = info.valid;
                }
            }else{
                write_file.close();
            }
        }
        //无效的就移除该文件
        if(!stitch_result){
            write_file.remove();
        }

        //setProcessProgress(file_count,success_count,fail_count);
        if(getProcessing()){
            emit stitchFinished(info);
        }
        //process=false表示人为终止
        setProcessing(false,stitch_result?1:-1);
    });
    taskWatcher.setFuture(future);
}

void EasyAudioTool::stitchUrlList(const QList<QUrl> &files, const QString &dstpath)
{
    stitchPathList(urlsToStrs(files),dstpath);
}

void EasyAudioTool::stop()
{
    onProcess = false;
}

void EasyAudioTool::setTargetFormat(int channels, int sampleRate, int sampleBit, const QString &codec)
{
    targetFormat.setChannelCount(channels);
    targetFormat.setSampleRate(sampleRate);
    //TODO sampleBit待定，可能需要定义一个枚举值
    {
        targetFormat.setSampleSize(16); sampleBit;
        targetFormat.setSampleType(QAudioFormat::SignedInt);
    }
    //TODO codec目前只支持转为wav-pcm
    targetFormat.setCodec(codec);
}
