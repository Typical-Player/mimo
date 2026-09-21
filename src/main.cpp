#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) {
    qputenv("QT_MEDIA_BACKEND", "ffmpeg");
    qputenv("QT_FFMPEG_DECODING_HW_DEVICE_TYPES", "");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.loadFromModule("mimo", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
