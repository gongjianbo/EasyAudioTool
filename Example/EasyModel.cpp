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

    connect(&audioTool,&EasyAudioTool::parseFinished,
            this,[this](const EasyAudioInfo &info){
        ModelItem item;
        item.info=info;
        appendAudio(item);
    });
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
    case Qt::UserRole+3: //md5
        return audioList.at(row).info.filemd5;
    case Qt::UserRole+4: //format
        return audioList.at(row).info.format;
    case Qt::UserRole+5: //encode
        return audioList.at(row).info.encode;
    case Qt::UserRole+6: //channels
        return audioList.at(row).info.channels;
    case Qt::UserRole+7: //sampleRate
        return audioList.at(row).info.sampleRate;
    case Qt::UserRole+8: //sampleBit
        return audioList.at(row).info.sampleBit;
    case Qt::UserRole+9: //bitRate
        return audioList.at(row).info.bitRate;
    case Qt::UserRole+10: //duration
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
        ,{ Qt::UserRole+3, "filemd5" }
        ,{ Qt::UserRole+4, "format" }
        ,{ Qt::UserRole+5, "encode" }
        ,{ Qt::UserRole+6, "channels" }
        ,{ Qt::UserRole+7, "sampleRate" }
        ,{ Qt::UserRole+8, "sampleBit" }
        ,{ Qt::UserRole+9, "bitRate" }
        ,{ Qt::UserRole+10, "duration" }
    };
}

void EasyModel::parseUrl(const QUrl &fileurl, const QStringList &filter)
{
    parseDir(fileurl.toLocalFile(),filter);
}

void EasyModel::parseDir(const QString &filedir, const QStringList &filter)
{
    clearAudio();
    audioTool.parseDirPath(filedir,!filter.isEmpty(),filter);
}

void EasyModel::clearAudio()
{
    beginResetModel();
    audioList.clear();
    endResetModel();
}

void EasyModel::appendAudio(const ModelItem &data)
{
    beginInsertRows(QModelIndex(),audioList.count(),audioList.count());
    audioList.push_back(data);
    endInsertRows();
}
