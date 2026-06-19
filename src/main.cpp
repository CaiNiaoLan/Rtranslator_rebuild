#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "app/MainWindow.h"
#include "app/ThemeManager.h"

int main(int argc, char *argv[])
{
    qputenv("QSG_RENDER_LOOP", "basic");
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    QApplication app(argc, argv);
    app.setOrganizationName("RTranslator");
    app.setApplicationName("RTranslator");

    ThemeManager themeManager;
    QQmlApplicationEngine engine;
    MainWindow mainWindow(&engine);
    engine.rootContext()->setContextProperty("mainWindow", &mainWindow);
    engine.rootContext()->setContextProperty("theme", &themeManager);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                qCritical() << "Failed to load QML:" << url;
                QCoreApplication::exit(-1);
            }
        }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}
