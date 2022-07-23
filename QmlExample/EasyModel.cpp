#include "EasyModel.h"
#include "EasyAudioFactory.h"

#include <QCoreApplication>
#include <QtConcurrentRun>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QTimer>
#include <QTime>
#include <QDebug>

EasyModel::EasyModel(QObject *parent)
    : QAbstractListModel(parent)
{
    qRegisterMetaType<ModelItem>("ModelItem");
    qRegisterMetaType<QList<ModelItem>>("QList<ModelItem>");

    connect(this,&EasyModel::parseOneFinished,this,&EasyModel::appendOneInfo);
    connect(&taskWatcher,&QFutureWatcher<void>::started,this,[=]{
        //setTaskRunning(true);
    });
    connect(&taskWatcher,&QFutureWatcher<void>::finished,this,[=]{
        setTaskRunning(false);
    });

    //启动后延迟加载目录下的音频
    QTimer::singleShot(1000,[this]{
        parseDir(qApp->applicationDirPath()+"/audio");
    });
}

EasyModel::~EasyModel()
{
    freeTask();
}

int EasyModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return audioList.count();
}

QVariant EasyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int row=index.row();
    const ModelItem &item=audioList.at(row);
    //类型主要为QString,int64,int
    switch(role)
    {
    case FilepathRole:
        return item.info.filepath;
    case FilenameRole:
        return item.info.filename;
    case FilesizeRole:
        return item.info.filesize;
    case MD5Role:
        return item.info.md5;
    case FormatRole:
        return item.info.format;
    case CodecRole:
        return item.info.codec;
    case EncodeRole:
        return item.info.encode;
    case ChannelsRole:
        return item.info.channels;
    case SampleRateRole:
        return item.info.sampleRate;
    case SampleBitRole:
        return item.info.sampleBit;
    case BitRateRole:
        return item.info.bitRate;
    case DurationRole:
        return item.info.duration;
    default:break;
    }

    return QVariant();
}

QHash<int, QByteArray> EasyModel::roleNames() const
{
    return QHash<int,QByteArray>{
        { FilepathRole, "filepath" }
        ,{ FilenameRole, "filename" }
        ,{ FilesizeRole, "filesize" }
        ,{ MD5Role, "md5" }
        ,{ FormatRole, "format" }
        ,{ CodecRole, "codec" }
        ,{ EncodeRole, "encode" }
        ,{ ChannelsRole, "channels" }
        ,{ SampleRateRole, "sampleRate" }
        ,{ SampleBitRole, "sampleBit" }
        ,{ BitRateRole, "bitRate" }
        ,{ DurationRole, "duration" }
    };
}

bool EasyModel::getTaskRunning() const
{
    return taskRunning;
}

void EasyModel::setTaskRunning(bool running)
{
    if(taskRunning!=running){
        taskRunning=running;
        emit taskRunningChanged(taskRunning);
    }
}

void EasyModel::freeTask()
{
    taskRunFlag=false;
    if(taskWatcher.isRunning()){
        taskWatcher.waitForFinished();
    }
}

void EasyModel::parseUrl(const QUrl &fileurl, const QStringList &filter)
{
    parseDir(fileurl.toLocalFile(),filter);
}

void EasyModel::parseDir(const QString &filedir, const QStringList &filter)
{
    //停止当前任务
    freeTask();
    //显示busy
    setTaskRunning(true);
    //清空列表
    clearInfo();
    //解析目录下音频
    QFuture<void> future=QtConcurrent::run([=]{
        taskRunFlag=true;
        QDir dir(filedir);
        if(dir.exists()){
            //先过滤文件列表
            const QFileInfoList info_list = dir.entryInfoList(filter,QDir::Files|QDir::NoSymLinks);
            QList<QString> file_list;
            for(auto &info : info_list)
            {
                file_list.push_back(info.filePath());
            }
            for(int index=0; index<file_list.count(); index++)
            {
                if(!taskRunFlag){
                    break;
                }
                const QString &filepath = file_list.at(index);

                //非文件、或者文件不存在不放到失败列表
                if(!QFileInfo(filepath).isFile()){
                    continue;
                }
                //提取音频信息
                QSharedPointer<EasyAbstractContext> p_context = EasyAudioFactory::createContext(filepath);
                if(p_context && p_context->isValid()){
                    EasyAudioInfo info = p_context->audioInfo();
                    if(info.valid){
                        ModelItem item;
                        item.info=info;
                        emit parseOneFinished(item);
                        continue;
                    }
                }
            }
        }else{
            qDebug()<<"parse dirpath not exists.";
        }
        taskRunFlag=false;
        qDebug()<<"parse dir finished."<<filedir<<filter;
    });
    taskWatcher.setFuture(future);
}

void EasyModel::clearInfo()
{
    beginResetModel();
    audioList.clear();
    endResetModel();
}

void EasyModel::appendOneInfo(const ModelItem &data)
{
    beginInsertRows(QModelIndex(),audioList.count(),audioList.count());
    audioList.push_back(data);
    endInsertRows();
}
