#include <QApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include "EasyAudioTool.h"
#include "EasyModel.h"
#include "EasyTest.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("EasyAudioTool");
    QCoreApplication::setOrganizationDomain("https://blog.csdn.net/gongjianbo1992");
    QCoreApplication::setApplicationName("EasyAudioTool");

    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    //注册qml组件
    EasyAudioTool::registerQmlType(&engine);
    EasyAudioTool::registerMetaType();
    qmlRegisterType<EasyModel>("EasyAudioTool",1,0,"EasyModel");
    qmlRegisterType<EasyTest>("EasyAudioTool",1,0,"EasyTest");

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
