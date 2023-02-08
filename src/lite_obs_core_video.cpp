#include "lite_obs_core_video.h"
#include "obs-defs.h"
#include "graphics/gs_subsystem.h"
#include "graphics/gs_texture.h"
#include "graphics/gs_stagesurf.h"
#include "media-io/video_output.h"
#include "media-io/video-matrices.h"
#include "util/log.h"
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <atomic>
#include <thread>
#include <mutex>
struct lite_obs_core_video_private
{
    std::unique_ptr<graphics_subsystem> graphics{};

    std::shared_ptr<gs_stagesurface> copy_surfaces[NUM_TEXTURES][NUM_CHANNELS]{};
    std::shared_ptr<gs_texture> render_texture{};
    std::shared_ptr<gs_texture> output_texture{};
    std::shared_ptr<gs_texture> convert_textures[NUM_CHANNELS]{};

    std::weak_ptr<gs_stagesurface> mapped_surfaces[NUM_CHANNELS];

    std::mutex gpu_encoder_mutex;

    uint64_t video_time{};
    uint64_t video_frame_interval_ns{};
    uint64_t video_avg_frame_time_ns{};
    double video_fps{};
    std::shared_ptr<video_output> video{};
    std::thread video_thread{};
    uint32_t total_frames{};
    uint32_t lagged_frames{};
    bool thread_initialized{};

    bool gpu_conversion{};
    const char *conversion_techs[NUM_CHANNELS]{};
    bool conversion_needed{};
    float conversion_width_i{};

    video_format output_format{};
    uint32_t output_width{};
    uint32_t output_height{};
    uint32_t base_width{};
    uint32_t base_height{};
    float color_matrix[16]{};
    obs_scale_type scale_type{};

    std::atomic_long raw_active{};
    std::atomic_long gpu_encoder_active{};

    obs_video_info ovi{};
};

lite_obs_core_video::lite_obs_core_video()
{
    d_ptr = std::make_unique<lite_obs_core_video_private>();
}

lite_obs_core_video::~lite_obs_core_video()
{

}

void lite_obs_core_video::lite_obs_video_free_data()
{
    if (d_ptr->video) {
        d_ptr->video->video_output_close();
        d_ptr->video.reset();
    }

    if (!d_ptr->graphics)
        return;

    gs_enter_contex(d_ptr->graphics);

    for (size_t c = 0; c < NUM_CHANNELS; c++) {
        auto surface = d_ptr->mapped_surfaces[c].lock();
        if (surface) {
            surface->gs_stagesurface_unmap();
            d_ptr->mapped_surfaces[c].reset();
        }
    }

    clear_gpu_copy_surface();
    d_ptr->render_texture.reset();
    clear_gpu_conversion_textures();

    d_ptr->output_texture.reset();

    gs_leave_context();
}

void lite_obs_core_video::graphics_thread(void *param)
{
    lite_obs_core_video *p = (lite_obs_core_video *)param;
    p->graphics_thread_internal();
}

void lite_obs_core_video::set_video_matrix(obs_video_info *ovi)
{
    glm::mat4x4 mat{0};
    glm::vec4 r_row{0};

    if (format_is_yuv(ovi->output_format)) {
        video_format_get_parameters(ovi->colorspace, ovi->range, (float *)&mat, NULL, NULL);
        mat = glm::inverse(mat);

        /* swap R and G */

        r_row = mat[0];
        mat[0] = mat[1];
        mat[1] = r_row;
    } else {
        mat = glm::mat4x4{1};
    }

    memcpy(d_ptr->color_matrix, &mat, sizeof(float) * 16);
}

void lite_obs_core_video::calc_gpu_conversion_sizes()
{
    d_ptr->conversion_needed = false;
    d_ptr->conversion_techs[0] = NULL;
    d_ptr->conversion_techs[1] = NULL;
    d_ptr->conversion_techs[2] = NULL;
    d_ptr->conversion_width_i = 0.f;

    switch (d_ptr->output_format) {
    case video_format::VIDEO_FORMAT_I420:
        d_ptr->conversion_needed = true;
        d_ptr->conversion_techs[0] = "Planar_Y";
        d_ptr->conversion_techs[1] = "Planar_U_Left";
        d_ptr->conversion_techs[2] = "Planar_V_Left";
        d_ptr->conversion_width_i = 1.f / (float)d_ptr->output_width;
        break;
    case video_format::VIDEO_FORMAT_NV12:
        d_ptr->conversion_needed = true;
        d_ptr->conversion_techs[0] = "NV12_Y";
        d_ptr->conversion_techs[1] = "NV12_UV";
        d_ptr->conversion_width_i = 1.f / (float)d_ptr->output_width;
        break;
    case video_format::VIDEO_FORMAT_I444:
        d_ptr->conversion_needed = true;
        d_ptr->conversion_techs[0] = "Planar_Y";
        d_ptr->conversion_techs[1] = "Planar_U";
        d_ptr->conversion_techs[2] = "Planar_V";
        break;
    default:
        break;
    }
}

void lite_obs_core_video::clear_gpu_conversion_textures()
{
    for (int i = 0; i < NUM_CHANNELS; ++i) {
        d_ptr->convert_textures[i].reset();
    }
}

bool lite_obs_core_video::init_gpu_conversion()
{
    calc_gpu_conversion_sizes();

    d_ptr->convert_textures[0] = gs_texture_create(d_ptr->output_width, d_ptr->output_height, gs_color_format::GS_R8, 1, NULL, GS_RENDER_TARGET);

    switch (d_ptr->output_format) {
    case video_format::VIDEO_FORMAT_I420:
        d_ptr->convert_textures[1] = gs_texture_create(d_ptr->output_width / 2, d_ptr->output_height / 2, gs_color_format::GS_R8, 1, NULL, GS_RENDER_TARGET);
        d_ptr->convert_textures[2] = gs_texture_create(d_ptr->output_width / 2, d_ptr->output_height / 2, gs_color_format::GS_R8, 1, NULL, GS_RENDER_TARGET);
        if (!d_ptr->convert_textures[2])
            return false;
        break;
    case video_format::VIDEO_FORMAT_NV12:
        d_ptr->convert_textures[1] = gs_texture_create(d_ptr->output_width / 2, d_ptr->output_height / 2, gs_color_format::GS_R8G8, 1, NULL, GS_RENDER_TARGET);
        break;
    case video_format::VIDEO_FORMAT_I444:
        d_ptr->convert_textures[1] = gs_texture_create(d_ptr->output_width, d_ptr->output_height, gs_color_format::GS_R8, 1, NULL, GS_RENDER_TARGET);
        d_ptr->convert_textures[2] = gs_texture_create(d_ptr->output_width, d_ptr->output_height, gs_color_format::GS_R8, 1, NULL, GS_RENDER_TARGET);
        if (!d_ptr->convert_textures[2])
            return false;
        break;
    default:
        break;
    }

    if (!d_ptr->convert_textures[0])
        return false;
    if (!d_ptr->convert_textures[1])
        return false;

    return true;
}

void lite_obs_core_video::clear_gpu_copy_surface()
{
    for (size_t i = 0; i < NUM_TEXTURES; i++) {
        for (size_t c = 0; c < NUM_CHANNELS; c++) {
            if (d_ptr->copy_surfaces[i][c]) {
                d_ptr->copy_surfaces[i][c].reset();
            }
        }
    }
}

bool lite_obs_core_video::init_gpu_copy_surface(size_t i)
{
    d_ptr->copy_surfaces[i][0] = gs_stagesurface_create(d_ptr->output_width, d_ptr->output_height, gs_color_format::GS_R8);
    if (!d_ptr->copy_surfaces[i][0])
        return false;

    switch (d_ptr->output_format) {
    case video_format::VIDEO_FORMAT_I420:
        d_ptr->copy_surfaces[i][1] = gs_stagesurface_create(d_ptr->output_width / 2, d_ptr->output_height / 2, gs_color_format::GS_R8);
        if (!d_ptr->copy_surfaces[i][1])
            return false;
        d_ptr->copy_surfaces[i][2] = gs_stagesurface_create(d_ptr->output_width / 2, d_ptr->output_height / 2, gs_color_format::GS_R8);
        if (!d_ptr->copy_surfaces[i][2])
            return false;
        break;
    case video_format::VIDEO_FORMAT_NV12:
        d_ptr->copy_surfaces[i][1] = gs_stagesurface_create(d_ptr->output_width / 2, d_ptr->output_height / 2, gs_color_format::GS_R8G8);
        if (!d_ptr->copy_surfaces[i][1])
            return false;
        break;
    case video_format::VIDEO_FORMAT_I444:
        d_ptr->copy_surfaces[i][1] = gs_stagesurface_create(d_ptr->output_width, d_ptr->output_height, gs_color_format::GS_R8);
        if (!d_ptr->copy_surfaces[i][1])
            return false;
        d_ptr->copy_surfaces[i][2] = gs_stagesurface_create(d_ptr->output_width, d_ptr->output_height, gs_color_format::GS_R8);
        if (!d_ptr->copy_surfaces[i][2])
            return false;
        break;
    default:
        break;
    }

    return true;
}

bool lite_obs_core_video::init_textures()
{
    for (size_t i = 0; i < NUM_TEXTURES; i++) {
        if (d_ptr->gpu_conversion) {
            if (!init_gpu_copy_surface(i)) {
                clear_gpu_copy_surface();
                return false;
            }
        } else {
            d_ptr->copy_surfaces[i][0] = gs_stagesurface_create(d_ptr->output_width, d_ptr->output_height, gs_color_format::GS_RGBA);
            if (!d_ptr->copy_surfaces[i][0]) {
                return false;
            }
        }
    }

    d_ptr->render_texture = gs_texture_create(d_ptr->base_width, d_ptr->base_height, gs_color_format::GS_RGBA, 1, NULL, GS_RENDER_TARGET);

    if (!d_ptr->render_texture)
        return false;

    d_ptr->output_texture = gs_texture_create(d_ptr->output_width, d_ptr->output_height, gs_color_format::GS_RGBA, 1, NULL, GS_RENDER_TARGET);

    if (!d_ptr->output_texture)
        return false;

    return true;
}

void lite_obs_core_video::graphics_thread_internal()
{

}

int lite_obs_core_video::lite_obs_init_graphics()
{
    if (d_ptr->graphics)
        return GS_SUCCESS;

    auto gs = std::make_unique<graphics_subsystem>();
    auto errcode = gs->gs_create();
    if (errcode != GS_SUCCESS) {
        blog(LOG_DEBUG, "lite_obs_core_video_init fail.");
        return errcode;
    }

    gs_enter_contex(gs);
    auto res = gs->gs_effect_init();
    if (res != GS_SUCCESS) {
        gs_leave_context();
        return GS_ERROR_FAIL;
    }
    gs_leave_context();

    d_ptr->graphics = std::move(gs);
    return GS_SUCCESS;
}

void lite_obs_core_video::lite_obs_free_graphics()
{
    d_ptr->graphics.reset();
}

static inline void make_video_info(video_output_info *vi, obs_video_info *ovi)
{
    vi->name = "video";
    vi->format = ovi->output_format;
    vi->fps_num = ovi->fps_num;
    vi->fps_den = ovi->fps_den;
    vi->width = ovi->output_width;
    vi->height = ovi->output_height;
    vi->range = ovi->range;
    vi->colorspace = ovi->colorspace;
    vi->cache_size = 6;
}
int lite_obs_core_video::lite_obs_core_video_init(obs_video_info *ovi)
{
    video_output_info vi;
    make_video_info(&vi, ovi);

    d_ptr->output_format = ovi->output_format;
    d_ptr->base_width = ovi->base_width;
    d_ptr->base_height = ovi->base_height;
    d_ptr->output_width = ovi->output_width;
    d_ptr->output_height = ovi->output_height;
    d_ptr->gpu_conversion = ovi->gpu_conversion;
    d_ptr->scale_type = ovi->scale_type;

    set_video_matrix(ovi);

    auto video = std::make_shared<video_output>();
    auto errorcode = video->video_output_open(&vi);
    if (errorcode != VIDEO_OUTPUT_SUCCESS) {
        if (errorcode == VIDEO_OUTPUT_INVALIDPARAM) {
            blog(LOG_ERROR, "Invalid video parameters specified");
            return OBS_VIDEO_INVALID_PARAM;
        } else {
            blog(LOG_ERROR, "Could not open video output");
        }
        return OBS_VIDEO_FAIL;
    }

    d_ptr->video = video;

    gs_enter_contex(d_ptr->graphics);

    if (ovi->gpu_conversion && !init_gpu_conversion()) {
        clear_gpu_conversion_textures();
        gs_leave_context();
        return OBS_VIDEO_FAIL;
    }

    if (!init_textures()) {
        gs_leave_context();
        return OBS_VIDEO_FAIL;
    }

    gs_leave_context();

    d_ptr->video_thread = std::thread(lite_obs_core_video::graphics_thread, this);
    d_ptr->thread_initialized = true;
    d_ptr->ovi = *ovi;
    return OBS_VIDEO_SUCCESS;
}

bool lite_obs_core_video::lite_obs_video_active()
{
    return d_ptr->raw_active > 0 || d_ptr->gpu_encoder_active > 0;
}

void lite_obs_core_video::lite_obs_stop_video()
{
    if (!d_ptr->video)
        return;

    d_ptr->video->video_output_stop();

    if (d_ptr->thread_initialized) {
        if (d_ptr->video_thread.joinable()) {
            d_ptr->video_thread.join();
            d_ptr->thread_initialized = false;
        }
    }
}
