#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QTimer>
#include <QThread>
#include <thread>
#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QFile>
#include <QSGRendererInterface>
#include "lite_obs.h"
#include "graphics/gs_device.h"
#include "graphics/gs_texture.h"
#include "fboinsgrenderer.h"
#include "output/null_output.h"
#include "encoder/lite_ffmpeg_video_encoder.h"
#include "lite_obs_core_video.h"

extern std::shared_ptr<gs_texture> test_texture;

int main(int argc, char *argv[])
{
    qmlRegisterType<FboInSGRenderer>("com.ypp", 1, 0, "Render");

    qputenv("QSG_RENDER_LOOP", "basic");
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QGuiApplication app(argc, argv);

    QLoggingCategory::setFilterRules("qt.scenegraph.general=true");
    qSetMessagePattern("%{category}:%{message}");
    QQmlApplicationEngine engine;
    QQuickView view(&engine, nullptr);

    QObject obj;
    QObject::connect(&view, &QQuickView::sceneGraphInitialized, &obj, [](){
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
    }, Qt::DirectConnection);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    view.setSource(url);
    view.show();

    QTimer::singleShot(1000, [](){
        auto encoder = std::make_shared<lite_ffmpeg_video_encoder>(0);
        auto output = std::make_shared<null_output>();
        output->lite_obs_output_create();
        output->lite_obs_output_set_video_encoder(encoder);
        encoder->lite_obs_encoder_set_video(obs.obs_core_video()->core_video());
        output->lite_obs_output_set_media(obs.obs_core_video()->core_video(), nullptr);

        output->lite_obs_output_start();

        QThread::sleep(5);

        output->lite_obs_output_stop();
    });


    return app.exec();
}
