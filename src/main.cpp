#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "app/MainWindow.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("RTranslator");
    app.setApplicationName("RTranslator");

    QQmlApplicationEngine engine;
    MainWindow mainWindow(&engine);
    engine.rootContext()->setContextProperty("mainWindow", &mainWindow);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}
