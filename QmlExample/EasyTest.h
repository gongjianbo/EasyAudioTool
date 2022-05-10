#pragma once
#include <QObject>
#include <atomic>

//接口测试
class EasyTest : public QObject
{
    Q_OBJECT
public:
    explicit EasyTest(QObject *parent = nullptr);

    Q_INVOKABLE void transcodeRun(const QString &filepath);
    Q_INVOKABLE void transcodeCancel();

private:
    std::atomic_bool runFlag{ false };
};
