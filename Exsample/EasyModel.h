#ifndef EASYMODEL_H
#define EASYMODEL_H

#include <QAbstractListModel>
#include "EasyAudioDefine.h"

struct ModelItem
{
    EasyAudioInfo info;
};

//用于展示音频文件的model
class EasyModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit EasyModel(QObject *parent = nullptr);

    //Data
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void parseFinished(const QList<ModelItem> &data);

public slots:
    void parseUrl(const QUrl &fileurl);
    void parseDir(const QString &filedir);
    void setAudioList(const QList<ModelItem> &data);

private:
    QList<ModelItem> audioList;
};

#endif // EASYMODEL_H
