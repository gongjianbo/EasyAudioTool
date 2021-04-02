#include "EasyAudioTool.h"

#include "EasyAudioFactory.h"
#include <QCoreApplication>
#include <QtConcurrentRun>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QFuture>
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

void EasyAudioTool::setProcessing(bool on)
{
    if(onProcess != on){
        onProcess = on;
        emit processStatusChanged();
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

    setProcessing(true);
    emit processStarted(files.count());
    setProcessProgress(files.count(),0,0);
    //因为解析的速度比较快，所以暂时不emit进度值，除非文件数较多
    QFuture<void> future = QtConcurrent::run(&taskPool,[=]{
        //解析失败的列表
        QStringList failed_list;
        QStringList filter_list;
        if(useFilter){
            filter_list = filter.isEmpty()?getSupportedSuffixs():filter;
        }

        for(int index=0; index<files.count(); index++)
        {
            if(!getProcessing()){
                break;
            }
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
                    continue;
                }
            }

            failed_list.push_back(filepath);
        }

        if(getProcessing()){
            if(!failed_list.isEmpty())
                emit parseFailedChanged(failed_list);
        }
        //parse结果固定为true
        emit processFinished(true);
        setProcessing(false);
    });
    //connect(taskWatcher,&QFutureWatcher<void>::finished,this,[this]{});
    //在调用stFuture之前应建立connection以避免争用,Future可能在connection完成之前完成。
    taskWatcher.setFuture(future);
}

void EasyAudioTool::parseUrlList(const QList<QUrl> &files,
                                 bool useFilter,
                                 const QStringList &filter)
{
    QList<QString> file_list;
    for(const QUrl &dir : files)
    {
        file_list.push_back(dir.toLocalFile());
    }
    parsePathList(file_list,useFilter,filter);
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

void EasyAudioTool::stop()
{
    onProcess = false;
}
