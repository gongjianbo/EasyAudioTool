#include <QApplication>
#include <QQmlApplicationEngine>
#include <QSettings>

#include "EasyAudioRegister.h"
#include "EasyTest.h"
#include "EasyModel.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("EasyAudio");
    QCoreApplication::setOrganizationDomain("https://blog.csdn.net/gongjianbo1992");
    QCoreApplication::setApplicationName("EasyAudio");

    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    EasyAudioRegister::registerQmlType(&engine);
    EasyAudioRegister::registerMetaType();
    qmlRegisterType<EasyTest>("Test",1,0,"EasyTest");
    qmlRegisterType<EasyModel>("Test",1,0,"EasyModel");

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
