#include "EasyModel.h"

#include <thread>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QDebug>
#include "EasyAudioFactory.h"

EasyModel::EasyModel(QObject *parent)
    : QAbstractListModel(parent)
{
    qRegisterMetaType<ModelItem>("ModelItem");
    qRegisterMetaType<QList<ModelItem>>("QList<ModelItem>");

    connect(this,&EasyModel::parseFinished,this,&EasyModel::setAudioList);
    parseDir(qApp->applicationDirPath()+"/audio");
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
    //类型主要为QString,int64,int
    switch(role)
    {
    case Qt::UserRole+0: //filepath
        return audioList.at(row).info.filepath;
    case Qt::UserRole+1: //filename
        return audioList.at(row).info.filename;
    case Qt::UserRole+2: //size
        return audioList.at(row).info.filesize;
    case Qt::UserRole+3: //format
        return audioList.at(row).info.format;
    case Qt::UserRole+4: //encode
        return audioList.at(row).info.encode;
    case Qt::UserRole+5: //channels
        return audioList.at(row).info.channels;
    case Qt::UserRole+6: //sampleRate
        return audioList.at(row).info.sampleRate;
    case Qt::UserRole+7: //sampleBit
        return audioList.at(row).info.sampleBit;
    case Qt::UserRole+8: //bitRate
        return audioList.at(row).info.bitRate;
    case Qt::UserRole+9: //duration
        return audioList.at(row).info.duration;
    default:break;
    }

    return QVariant();
}

QHash<int, QByteArray> EasyModel::roleNames() const
{
    return QHash<int,QByteArray>{
        { Qt::UserRole+0, "filepath" }
        ,{ Qt::UserRole+1, "filename" }
        ,{ Qt::UserRole+2, "filesize" }
        ,{ Qt::UserRole+3, "format" }
        ,{ Qt::UserRole+4, "encode" }
        ,{ Qt::UserRole+5, "channels" }
        ,{ Qt::UserRole+6, "sampleRate" }
        ,{ Qt::UserRole+7, "sampleBit" }
        ,{ Qt::UserRole+8, "bitRate" }
        ,{ Qt::UserRole+9, "duration" }
    };
}

void EasyModel::parseUrl(const QUrl &fileurl)
{
    parseDir(fileurl.toLocalFile());
}

void EasyModel::parseDir(const QString &filedir)
{
    std::thread th([this,filedir]{
        QList<ModelItem> data;
        QDir dir(filedir);
        //列出dir(path)目录文件下所有文件
        QList<QFileInfo> file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        //列出dir(path)目录下所有子文件夹
        //QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        //进行子文件夹folder_list递归遍历，将内容存入file_list容器
        for(int i= 0; i < file_list.count(); i++)
        {
            //qDebug()<<file_list.at(i).filePath();
            QSharedPointer<EasyAbstractContext> context=
                    EasyAudioFactory::createContext(file_list.at(i).filePath());
            if(context&&context->isValid()){
                ModelItem item;
                item.info=context->audioInfo();
                data.push_back(item);
            }
        }
        emit parseFinished(data);
    });
    th.detach();
}

void EasyModel::setAudioList(const QList<ModelItem> &data)
{
    beginResetModel();
    audioList=data;
    endResetModel();
}
