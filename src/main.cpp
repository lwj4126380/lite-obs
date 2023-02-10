#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <thread>
#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QFile>
#include <QSGRendererInterface>
#include <QQuickWindow>
#include "lite_obs.h"
#include "graphics/gs_device.h"
#include "graphics/gs_texture.h"

extern std::shared_ptr<gs_texture> test_texture;

int main(int argc, char *argv[])
{
    qputenv("QSG_RENDER_LOOP", "basic");
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QGuiApplication app(argc, argv);

    QLoggingCategory::setFilterRules("qt.scenegraph.general=true");
    qSetMessagePattern("%{category}:%{message}");
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
//    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
//                     &app, [url](QObject *obj, const QUrl &objUrl) {
//        if (!obj && url == objUrl)
//            QCoreApplication::exit(-1);
//    }, Qt::QueuedConnection);
    engine.load(url);

    obs_video_info info;
    info.base_width = 1920;
    info.base_height = 1080;
    info.fps_den = 1;
    info.fps_num = 60;
    info.output_width = 1280;
    info.output_height = 720;
    info.output_format = video_format::VIDEO_FORMAT_NV12;
    info.gpu_conversion = true;
    info.colorspace = video_colorspace::VIDEO_CS_601;
    info.range = video_range_type::VIDEO_RANGE_PARTIAL;
    obs.obs_reset_video(&info);

    QTimer t;
    t.setSingleShot(true);
    QObject::connect(&t, &QTimer::timeout, [=](){

        QImage f(":/test.jpg");
        f = f.convertedTo(QImage::Format_RGBA8888);
        auto b = f.bits();
        obs.obs_enter_graphics_context();
        test_texture = gs_texture_create(f.width(), f.height(), gs_color_format::GS_RGBA, 1, (const uint8_t **)&b, GS_DYNAMIC);
        blog(LOG_DEBUG, test_texture ? "111111111111 " : " 2222222222222222");
        obs.obs_leave_graphics_context();
    });
    t.start(1000);

    QTimer::singleShot(3000, [](){
        qApp->quit();
    });


    return app.exec();
}
