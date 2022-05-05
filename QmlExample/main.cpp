#include <QApplication>
#include <QQmlApplicationEngine>
#include <QSettings>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("EasyAudioTool");
    QCoreApplication::setOrganizationDomain("https://blog.csdn.net/gongjianbo1992");
    QCoreApplication::setApplicationName("EasyAudioTool");

    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
