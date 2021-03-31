#include <QApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QFile>
#include <QDebug>
#include "EasyAudioRegister.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    //const QString path="D:/Temp/1.flac";
    //QSharedPointer<EasyFFmpegContext> cp(new EasyFFmpegContext(path));
    //EasyFFmpegDecoder dec;
    //dec.setContext(cp);
    //QAudioFormat fmt;
    //fmt.setCodec("audio/pcm");
    //fmt.setChannelCount(1);
    //fmt.setSampleRate(16000);
    //fmt.setSampleSize(16);
    //fmt.setSampleType(QAudioFormat::SignedInt);
    //if(dec.open(fmt)){
    //    QByteArray data;
    //    while(!dec.isEnd()){
    //        data+=dec.read(1024*1024);
    //    }
    //    //QByteArray data=dec.readAll();
    //    qDebug()<<"read data"<<data.size();
    //    EasyWavHead head=EasyWavHead::createHead(fmt,data.size());
    //    QFile file(path+".wav");
    //    if(file.open(QIODevice::WriteOnly)){
    //        file.write(QByteArray((char*)&head,sizeof(EasyWavHead)));
    //        file.write(data);
    //        file.close();
    //    }
    //
    //    dec.close();
    //}

    QQmlApplicationEngine engine;
    EasyAudioRegister::registerQmlType(&engine);
    EasyAudioRegister::registerMetaType();

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
