#include "lite_obs.h"
#include <mutex>
#include <list>

#include "graphics/gs_subsystem.h"
#include "graphics/gs_texture.h"
#include "graphics/gs_texture_render.h"
#include "graphics/gs_program.h"
#include "graphics/gs_stagesurf.h"

#include "util/log.h"
#include "obs-defs.h"
#include "lite_obs_core_video.h"

lite_obs obs;

struct lite_obs_data
{
    std::recursive_mutex sources_mutex;
    std::recursive_mutex audio_sources_mutex;

    std::list<std::shared_ptr<lite_source>> sources{};
    std::list<std::shared_ptr<lite_source>> audio_sources{};
};

struct lite_obs_private
{
    lite_obs_data data{};
    lite_obs_core_video video{};
};

lite_obs::lite_obs()
{
    d_ptr = std::make_unique<lite_obs_private>();
}

lite_obs::~lite_obs()
{
    obs_shutdown();
}

void lite_obs::add_source(std::shared_ptr<lite_source> source, bool is_audio_source)
{
    if (is_audio_source) {
        std::lock_guard<std::recursive_mutex> lock(d_ptr->data.audio_sources_mutex);
        d_ptr->data.audio_sources.push_back(source);
    }

    std::lock_guard<std::recursive_mutex> lock(d_ptr->data.sources_mutex);
    d_ptr->data.sources.push_back(source);
}

void lite_obs::remove_source(std::shared_ptr<lite_source> source)
{
    {
        std::lock_guard<std::recursive_mutex> lock(d_ptr->data.audio_sources_mutex);
        for (auto iter = d_ptr->data.audio_sources.begin(); iter != d_ptr->data.audio_sources.end(); iter++) {
            if (source == *iter) {
                d_ptr->data.audio_sources.erase(iter);
                break;
            }
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock(d_ptr->data.sources_mutex);
        for (auto iter = d_ptr->data.sources.begin(); iter != d_ptr->data.sources.end(); iter++) {
            if (source == *iter) {
                d_ptr->data.sources.erase(iter);
                break;
            }
        }
    }
}

#define OBS_SIZE_MIN 2
#define OBS_SIZE_MAX (32 * 1024)

static inline bool size_valid(uint32_t width, uint32_t height)
{
    return (width >= OBS_SIZE_MIN && height >= OBS_SIZE_MIN &&
        width <= OBS_SIZE_MAX && height <= OBS_SIZE_MAX);
}

int lite_obs::obs_reset_video(obs_video_info *ovi)
{
    if (d_ptr->video.lite_obs_video_active())
        return OBS_VIDEO_CURRENTLY_ACTIVE;

    if (!size_valid(ovi->output_width, ovi->output_height) ||
            !size_valid(ovi->base_width, ovi->base_height))
            return OBS_VIDEO_INVALID_PARAM;

    if (ovi->output_format != video_format::VIDEO_FORMAT_NV12
            && ovi->output_format != video_format::VIDEO_FORMAT_I420
            && ovi->output_format != video_format::VIDEO_FORMAT_I444) {
        blog(LOG_DEBUG, "video output format request one of (NV12 I420 I444)");
        return OBS_VIDEO_INVALID_PARAM;
    }

    d_ptr->video.lite_obs_stop_video();

    ovi->output_width &= 0xFFFFFFFC;
    ovi->output_height &= 0xFFFFFFFE;

    return d_ptr->video.lite_obs_start_video(ovi);
}

void lite_obs::obs_enter_graphics_context()
{
    gs_enter_contex(d_ptr->video.graphics());
}

void lite_obs::obs_leave_graphics_context()
{
    gs_leave_context();
}

void lite_obs::obs_shutdown()
{
    d_ptr->video.lite_obs_stop_video();
//    stop_audio();

//    obs_free_audio();
//    obs_free_data();

//    obs_rtc_capture_free(true);
//    obs_free_hotkeys();

//    proc_handler_destroy(obs->procs);
//    signal_handler_destroy(obs->signals);
//    obs->procs = NULL;
//    obs->signals = NULL;
}

#include <thread>
#include <QFile>
#include <QImage>
#include <QElapsedTimer>
#include <glm/vec4.hpp>
int lite_obs::obs_init_graphics()
{
    gs_enter_contex(d_ptr->video.graphics());
    auto render_texture = gs_texture_create(1920, 1080, gs_color_format::GS_RGBA, 1, NULL, GS_RENDER_TARGET);

    glm::vec4 clear_color(0);
    gs_set_render_target(render_texture, NULL);
    gs_clear(GS_CLEAR_COLOR, &clear_color, 1.0f, 0);

    gs_set_render_size(1920, 1080);



        QImage f(":/test.jpg");
        f = f.convertedTo(QImage::Format_RGBA8888);
        auto b = f.bits();
        auto test_texture = gs_texture_create(f.width(), f.height(), gs_color_format::GS_RGBA, 1, (const uint8_t **)&b, GS_DYNAMIC);


        auto program = d_ptr->video.graphics()->gs_get_effect_by_name("Default_Draw");
        gs_set_cur_effect(program);

        gs_technique_begin();
        program->gs_effect_set_texture("image", test_texture);
        d_ptr->video.graphics()->gs_draw_sprite(test_texture, 0, test_texture->gs_texture_get_width(), test_texture->gs_texture_get_height());
        gs_technique_end();

        auto ss = gs_stagesurface_create(1920, 1080, gs_color_format::GS_RGBA);
        ss->gs_stagesurface_stage_texture(render_texture);
        uint8_t *d;
        uint32_t lin;
        ss->gs_stagesurface_map(&d, &lin);
        QFile ff("/storage/emulated/0/Android/data/org.qtproject.example.lite_obs/files/cccc.rgba");
        ff.open(QFile::ReadWrite);
        ff.write((char *)d, 1920 * 1080 * 4);
        ff.close();
        ss->gs_stagesurface_unmap();
//    gs_enter_contex(d_ptr->video.graphics());
//        auto target = std::make_shared<gs_texture_render>(gs_color_format::GS_RGBA, gs_zstencil_format::GS_ZS_NONE);
//        target->gs_texrender_reset();

//        target->gs_texrender_begin(640, 360);

//        QFile file(":/640-360-420.yuv");
//        file.open(QFile::ReadOnly);
//        QByteArray data = file.readAll();
//        file.close();

//        auto y_tex = gs_texture_create(640, 360, gs_color_format::GS_R8, 1, nullptr, GS_DYNAMIC);
//        auto u_tex = gs_texture_create(320, 180, gs_color_format::GS_R8, 1, nullptr, GS_DYNAMIC);
//        auto v_tex = gs_texture_create(320, 180, gs_color_format::GS_R8, 1, nullptr, GS_DYNAMIC);

//        y_tex->gs_texture_set_image((const uint8_t *)data.data(), 640, false);
//        u_tex->gs_texture_set_image((const uint8_t *)(data.data()+640*360), 320, false);
//        v_tex->gs_texture_set_image((const uint8_t *)(data.data()+640*360 + 640*360/4), 320, false);

//        auto program = d_ptr->video.graphics()->gs_get_effect_by_name("Convert_I420_Reverse");
//        gs_set_cur_effect(program);

//        float float_range_min[3] = {16.0f / 255.0f, 16.0f / 255.0f, 16.0f / 255.0f};
//        float float_range_max[3] = {235.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f};
//        float matrix[2][16] = {{1.164384f, 0.000000f, 1.596027f, -0.874202f, 1.164384f, -0.391762f,
//                                -0.812968f, 0.531668f, 1.164384f, 2.017232f, 0.000000f, -1.085631f,
//                                0.000000f, 0.000000f, 0.000000f, 1.000000f},
//                               {1.000000f, 0.000000f, 1.407520f, -0.706520f, 1.000000f, -0.345491f,
//                                -0.716948f, 0.533303f, 1.000000f, 1.778976f, 0.000000f, -0.892976f,
//                                0.000000f, 0.000000f, 0.000000f, 1.000000f}};

//        glm::vec4 vec0 = {matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3]};
//        glm::vec4 vec1 = {matrix[0][4], matrix[0][5], matrix[0][6], matrix[0][7]};
//        glm::vec4 vec2 = {matrix[0][8], matrix[0][9], matrix[0][10], matrix[0][11]};

//        gs_enable_blending(false);

//        gs_technique_begin();

//        program->gs_effect_set_texture("image", y_tex);
//        program->gs_effect_set_texture("image1", u_tex);
//        program->gs_effect_set_texture("image2", v_tex);

//        float cx = 640.0f;
//        float cy = 360.0f;
//        program->gs_effect_set_param("width", cx);
//        program->gs_effect_set_param("height", cy);
//        program->gs_effect_set_param("width_d2", cx * 0.5f);
//        program->gs_effect_set_param("height_d2", cy * 0.5f);
//        program->gs_effect_set_param("width_x2_i", 0.5f / cx);

//        program->gs_effect_set_param("color_vec0", vec0);
//        program->gs_effect_set_param("color_vec1", vec1);
//        program->gs_effect_set_param("color_vec2", vec2);

//        program->gs_effect_set_param("color_range_min", float_range_min, sizeof(float) * 3);
//        program->gs_effect_set_param("color_range_max", float_range_max, sizeof(float) * 3);

//        gs_draw(gs_draw_mode::GS_TRIS, 0, 3);

//        gs_technique_end();

//        gs_enable_blending(true);


////        auto program = gl->gs_get_effect_by_name("Draw");
////        gs_set_cur_effect(program);

////        program->gs_effect_set_texture("image", img_tex);

////        gl->gs_draw_sprite(img_tex, 0, 1005, 792);

//        glFlush();

//        auto copy = std::make_shared<gs_stagesurface>();
//        if (copy->gs_stagesurface_create(640, 360, gs_color_format::GS_RGBA)) {
//            copy->gs_stagesurface_stage_texture(target->gs_texrender_get_texture());
//            uint8_t *data = nullptr;
//            uint32_t linesize = 0;
//            if (copy->gs_stagesurface_map(&data, &linesize)) {

//                QFile f("600.rgba");
//                f.remove();
//                f.open(QFile::ReadWrite);
//                f.write((char *)data, 640*360*4);
//                f.close();

//                copy->gs_stagesurface_unmap();
//            }
//        }

//        target->gs_texrender_end();

//        copy.reset();
//        target.reset();
//        y_tex.reset();
//        u_tex.reset();
//        v_tex.reset();


//        gs_leave_context();

    return true;
}
