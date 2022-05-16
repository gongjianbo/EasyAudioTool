#pragma once
#include <QAbstractListModel>
#include <QFutureWatcher>
#include "EasyAudioCommon.h"
#include "EasyAudioTool.h"

struct ModelItem
{
    EasyAudioInfo info;
};

//用于展示音频文件的model
class EasyModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool taskRunning READ getTaskRunning NOTIFY taskRunningChanged)
private:
    //role对应data/roleNames等接口的role
    enum ModelRole
    {
        FilepathRole=Qt::UserRole //文件路径
        ,FilenameRole //文件名
        ,FilesizeRole //文件大小byte
        ,MD5Role //md5值
        ,FormatRole //格式
        ,CodecRole //解码器
        ,EncodeRole //编码
        ,ChannelsRole //通道数
        ,SampleRateRole //采样率
        ,SampleBitRole //位宽
        ,BitRateRole //比特率
        ,DurationRole //时长ms
    };
public:
    explicit EasyModel(QObject *parent = nullptr);
    ~EasyModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool getTaskRunning() const;
    void setTaskRunning(bool running);

signals:
    void parseOneFinished(const ModelItem &data);
    void taskRunningChanged(bool running);

public slots:
    //结束任务
    void freeTask();
    //获取文件信息
    void parseUrl(const QUrl &fileurl, const QStringList &filter = QStringList());
    void parseDir(const QString &filedir, const QStringList &filter = QStringList());

    //model item增删
    void clearInfo();
    void appendOneInfo(const ModelItem &data);

private:
    //列表中的音频信息
    QList<ModelItem> audioList;
    //线程任务
    QFutureWatcher<void> taskWatcher;
    std::atomic_bool taskRunFlag{false};
    bool taskRunning{false};
};
