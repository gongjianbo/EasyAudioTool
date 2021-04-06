#pragma once
#include <QObject>
#include "EasyAudioPlayer.h"

//本地测试
class EasyTest : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

public slots:
    //【】
    void testDecoder();

    void tFFmpegRead(const QString &filepath);
    void tFFmpegReadAll(const QString &filepath);
    void tFFmpegCallBack(const QString &filepath);

    void tSilkRead(const QString &filepath);
    void tSilkReadAll(const QString &filepath);
    void tSilkCallBack(const QString &filepath);

    void tFactoryContext(const QString &filepath);
    void tFactoryDecoder(const QString &filepath);

    //【】
    void testPlayer();

private:
    EasyAudioPlayer player;
};

