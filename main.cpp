#include <QApplication>
#include <QQmlApplicationEngine>
#include <QSettings>

#include "EasyAudioRegister.h"
#include "EasyTest.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    EasyAudioRegister::registerQmlType(&engine);
    EasyAudioRegister::registerMetaType();
    EasyTest test;
    test.TestAll();

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
