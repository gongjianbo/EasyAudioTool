#ifndef EASYMODEL_H
#define EASYMODEL_H

#include <QAbstractListModel>
#include "EasyAudioDefine.h"
#include "EasyAudioTool.h"

struct ModelItem
{
    EasyAudioInfo info;
};

//用于展示音频文件的model
class EasyModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(EasyAudioTool *tool READ getTool CONSTANT)
public:
    explicit EasyModel(QObject *parent = nullptr);

    //Data
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    EasyAudioTool *getTool() { return &audioTool; }

public slots:
    //获取文件信息
    void parseUrl(const QUrl &fileurl, const QStringList &filter = QStringList());
    void parseDir(const QString &filedir, const QStringList &filter = QStringList());
    //转码
    void transcodeAll();
    //拼接
    void stitchAll();
    //model item增删
    void clearAudio();
    void appendAudio(const ModelItem &data);

private:
    QList<ModelItem> audioList;
    EasyAudioTool audioTool;
};

#endif // EASYMODEL_H
