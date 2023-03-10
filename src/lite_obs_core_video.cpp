#include "lite_obs_core_video.h"
#include "obs-defs.h"
#include "graphics/gs_subsystem.h"
#include "graphics/gs_texture.h"
#include "graphics/gs_stagesurf.h"
#include "graphics/gs_program.h"
#include "graphics/gs_device.h"
#include "media-io/video_output.h"
#include "media-io/video-matrices.h"
#include "util/log.h"
#include "util/threading.h"
#include "util/circlebuf.h"
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <atomic>
#include <thread>
#include <mutex>

struct obs_vframe_info {
    uint64_t timestamp{};
    int count{};
};

struct obs_graphics_context {
    uint64_t last_time{};
    uint64_t interval{};
    uint64_t frame_time_total_ns{};
    uint64_t fps_total_ns{};
    uint32_t fps_total_frames{};
    bool gpu_was_active{};
    bool raw_was_active{};
    bool was_active{};
};

struct lite_obs_core_video_private
{
    std::unique_ptr<graphics_subsystem> graphics{};

    std::shared_ptr<gs_stagesurface> copy_surfaces[NUM_TEXTURES][NUM_CHANNELS]{};
    std::shared_ptr<gs_texture> render_texture{};
    std::shared_ptr<gs_texture> output_texture{};
    std::shared_ptr<gs_texture> convert_textures[NUM_CHANNELS]{};

    bool texture_rendered{};
    bool textures_copied[NUM_TEXTURES]{};
    bool texture_converted{};
    bool using_nv12_tex{};
    circlebuf vframe_info_buffer{};
    circlebuf vframe_info_buffer_gpu{};

    int cur_texture{};

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

void lite_obs_core_video::lite_obs_core_video_change_raw_active(bool add)
{
    if (add)
        d_ptr->raw_active++;
    else
        d_ptr->raw_active--;
}

void lite_obs_core_video::clear_base_frame_data(void)
{
    d_ptr->texture_rendered = false;
    d_ptr->texture_converted = false;
    circlebuf_free(&d_ptr->vframe_info_buffer);
    d_ptr->cur_texture = 0;
}

void lite_obs_core_video::clear_raw_frame_data(void)
{
    memset(d_ptr->textures_copied, 0, sizeof(d_ptr->textures_copied));
    circlebuf_free(&d_ptr->vframe_info_buffer);
}

void lite_obs_core_video::clear_gpu_frame_data(void)
{
    circlebuf_free(&d_ptr->vframe_info_buffer_gpu);
}

void lite_obs_core_video::video_sleep(bool raw_active, const bool gpu_active, uint64_t *p_time, uint64_t interval_ns)
{
    obs_vframe_info vframe_info;
    uint64_t cur_time = *p_time;
    uint64_t t = cur_time + interval_ns;
    int count;

    if (os_sleepto_ns(t)) {
        *p_time = t;
        count = 1;
    } else {
        count = (int)((os_gettime_ns() - cur_time) / interval_ns);
        *p_time = cur_time + interval_ns * count;
    }

    d_ptr->total_frames += count;
    d_ptr->lagged_frames += count - 1;

    vframe_info.timestamp = cur_time;
    vframe_info.count = count;

    if (raw_active)
        circlebuf_push_back(&d_ptr->vframe_info_buffer, &vframe_info, sizeof(vframe_info));

    if (gpu_active)
        circlebuf_push_back(&d_ptr->vframe_info_buffer_gpu, &vframe_info, sizeof(vframe_info));
}

std::shared_ptr<gs_texture> test_texture;
//#include <QFile>
//#include <QImage>
void lite_obs_core_video::render_all_sources()
{
    if (!test_texture) {
//        QImage f(":/test.jpg");
//        f = f.convertedTo(QImage::Format_RGBA8888);
//        auto b = f.bits();
//        test_texture = gs_texture_create(f.width(), f.height(), gs_color_format::GS_RGBA, 1, (const uint8_t **)&b, GS_DYNAMIC);
//        blog(LOG_DEBUG, test_texture ? "111111111111 " : " 2222222222222222");
        return;
    }

    auto program = d_ptr->graphics->gs_get_effect_by_name("Default_Draw");
    gs_set_cur_effect(program);

    gs_technique_begin();
    program->gs_effect_set_texture("image", test_texture);
    d_ptr->graphics->gs_draw_sprite(test_texture, 0, test_texture->gs_texture_get_width(), test_texture->gs_texture_get_height());
    gs_technique_end();
}

void lite_obs_core_video::render_main_texture()
{
    glm::vec4 clear_color(0);
    gs_set_render_target(d_ptr->render_texture, NULL);
    gs_clear(GS_CLEAR_COLOR, &clear_color, 1.0f, 0);

    gs_set_render_size(d_ptr->base_width, d_ptr->base_height);

    render_all_sources();

    d_ptr->texture_rendered = true;
}

bool lite_obs_core_video::resolution_close(uint32_t width, uint32_t height)
{
    long width_cmp = (long)d_ptr->base_width - (long)width;
    long height_cmp = (long)d_ptr->base_height - (long)height;

    return labs(width_cmp) <= 16 && labs(height_cmp) <= 16;
}

std::shared_ptr<gs_program> lite_obs_core_video::get_scale_effect(uint32_t width, uint32_t height)
{
    if (resolution_close(width, height)) {
        return d_ptr->graphics->gs_get_effect_by_name("Default_Draw");;
    } else {
        return d_ptr->graphics->gs_get_effect_by_name("Scale_Draw");;
    }
}

std::shared_ptr<gs_texture> lite_obs_core_video::render_output_texture()
{
    //here we remove RGBA output format support.
    auto texture = d_ptr->render_texture;
    auto target = d_ptr->output_texture;
    uint32_t width = target->gs_texture_get_width();
    uint32_t height = target->gs_texture_get_height();

    auto program = get_scale_effect(width, height);

    if ((program->gs_program_name() == "Default_Draw") && (width == d_ptr->base_width) && (height == d_ptr->base_height))
        return texture;

    gs_set_render_target(target, nullptr);
    gs_set_render_size(width, height);

    glm::vec2 base = {(float)d_ptr->base_width, (float)d_ptr->base_height};
    program->gs_effect_set_param("base_dimension", base);
    program->gs_effect_set_param("base_dimension_f", base);

    glm::vec2 base_i = {1.0f / (float)d_ptr->base_width, 1.0f / (float)d_ptr->base_height};
    program->gs_effect_set_param("base_dimension_i", base_i);

    program->gs_effect_set_texture("image", texture);

    gs_set_cur_effect(program);

    gs_enable_blending(false);

    gs_technique_begin();
    d_ptr->graphics->gs_draw_sprite(texture, 0, width, height);
    gs_technique_end();
    gs_enable_blending(true);

    return target;
}

void lite_obs_core_video::render_convert_plane(std::shared_ptr<gs_texture> target)
{
    const uint32_t width = target->gs_texture_get_width();
    const uint32_t height = target->gs_texture_get_height();

    gs_set_render_target(target, NULL);
    gs_set_render_size(width, height);

    gs_technique_begin();
    gs_draw(gs_draw_mode::GS_TRIS, 0, 3);
    gs_technique_end();
}

void lite_obs_core_video::render_convert_texture(std::shared_ptr<gs_texture> texture)
{
    gs_enable_blending(false);

    glm::vec4 vec0 = {d_ptr->color_matrix[4], d_ptr->color_matrix[5], d_ptr->color_matrix[6], d_ptr->color_matrix[7]};
    glm::vec4 vec1 = {d_ptr->color_matrix[0], d_ptr->color_matrix[1], d_ptr->color_matrix[2], d_ptr->color_matrix[3]};
    glm::vec4 vec2 = {d_ptr->color_matrix[8], d_ptr->color_matrix[9], d_ptr->color_matrix[10], d_ptr->color_matrix[11]};

    if (d_ptr->convert_textures[0]) {
        auto program = d_ptr->graphics->gs_get_effect_by_name(d_ptr->conversion_techs[0]);
        gs_set_cur_effect(program);
        program->gs_effect_set_param("color_vec0", vec0);
        program->gs_effect_set_texture("image", texture);
        render_convert_plane(d_ptr->convert_textures[0]);

        if (d_ptr->convert_textures[1]) {
            auto program1 = d_ptr->graphics->gs_get_effect_by_name(d_ptr->conversion_techs[1]);
            gs_set_cur_effect(program1);
            program1->gs_effect_set_param("color_vec1", vec1);
            program1->gs_effect_set_texture("image", texture);
            if (!d_ptr->convert_textures[2])
                program1->gs_effect_set_param("color_vec2", vec2);
            program1->gs_effect_set_param("width_i", d_ptr->conversion_width_i);
            render_convert_plane(d_ptr->convert_textures[1]);

            if (d_ptr->convert_textures[2]) {
                auto program2 = d_ptr->graphics->gs_get_effect_by_name(d_ptr->conversion_techs[2]);
                gs_set_cur_effect(program2);
                program2->gs_effect_set_param("color_vec1", vec1);
                program2->gs_effect_set_texture("image", texture);
                program2->gs_effect_set_param("color_vec2", vec2);
                program2->gs_effect_set_param("width_i", d_ptr->conversion_width_i);
                render_convert_plane(d_ptr->convert_textures[2]);
            }
        }
    }

    for (int i = 0; i < 3; ++i) {
        if (!d_ptr->convert_textures[i])
            continue;

        glm::vec4 vec0 = {d_ptr->color_matrix[4], d_ptr->color_matrix[5], d_ptr->color_matrix[6], d_ptr->color_matrix[7]};
        glm::vec4 vec1 = {d_ptr->color_matrix[0], d_ptr->color_matrix[1], d_ptr->color_matrix[2], d_ptr->color_matrix[3]};
        glm::vec4 vec2 = {d_ptr->color_matrix[8], d_ptr->color_matrix[9], d_ptr->color_matrix[10], d_ptr->color_matrix[11]};

        auto program = d_ptr->graphics->gs_get_effect_by_name(d_ptr->conversion_techs[i]);
        program->gs_effect_set_texture("image", texture);
        if (i == 0) {
            program->gs_effect_set_param("color_vec0", vec0);
        } else if (i == 1) {
            program->gs_effect_set_param("color_vec1", vec1);
            if (!d_ptr->convert_textures[i+1])
                program->gs_effect_set_param("color_vec2", vec2);
            program->gs_effect_set_param("width_i", d_ptr->conversion_width_i);
        } else if (i == 2) {
            program->gs_effect_set_param("color_vec2", vec2);
            program->gs_effect_set_param("width_i", d_ptr->conversion_width_i);
        }
    }

    gs_enable_blending(true);

    d_ptr->texture_converted = true;
}

void lite_obs_core_video::stage_output_texture(int cur_texture)
{
    for (int c = 0; c < NUM_CHANNELS; ++c) {
        auto surface = d_ptr->mapped_surfaces[c].lock();
        if (surface) {
            surface->gs_stagesurface_unmap();
            d_ptr->mapped_surfaces[c].reset();
        }
    }

    if (!d_ptr->gpu_conversion) {
        auto copy = d_ptr->copy_surfaces[cur_texture][0];
        if (copy)
            copy->gs_stagesurface_stage_texture(d_ptr->output_texture);

        d_ptr->textures_copied[cur_texture] = true;
    } else if (d_ptr->texture_converted) {
        for (int i = 0; i < NUM_CHANNELS; i++) {
            auto copy = d_ptr->copy_surfaces[cur_texture][i];
            if (copy)
                copy->gs_stagesurface_stage_texture(d_ptr->convert_textures[i]);
        }

        d_ptr->textures_copied[cur_texture] = true;
    }
}

void lite_obs_core_video::render_video(bool raw_active, const bool gpu_active, int cur_texture, int prev_texture)
{
    gs_begin_scene();

    gs_enable_depth_test(false);
    gs_set_cull_mode(gs_cull_mode::GS_NEITHER);

    render_main_texture();

    if (raw_active || gpu_active) {
        auto texture = render_output_texture();

#ifdef _WIN32
        if (gpu_active)
            gs_flush();
#endif

        if (d_ptr->gpu_conversion)
            render_convert_texture(texture);

#ifdef _WIN32
        if (gpu_active) {
            gs_flush();
            //output_gpu_encoders(video, raw_active); todo
        }
#endif

        if (raw_active)
            stage_output_texture(cur_texture);
    }

    gs_set_render_target(NULL, NULL);
    gs_enable_blending(true);

    gs_end_scene();
}

bool lite_obs_core_video::download_frame(int prev_texture, video_data *frame)
{
    if (!d_ptr->textures_copied[prev_texture])
        return false;

    for (int channel = 0; channel < NUM_CHANNELS; ++channel) {
        auto surface = d_ptr->copy_surfaces[prev_texture][channel];
        if (surface) {
            if (!surface->gs_stagesurface_map(&frame->frame.data[channel], &frame->frame.linesize[channel]))
                return false;

            d_ptr->mapped_surfaces[channel] = surface;
        }
    }
    return true;
}

static const uint8_t *set_gpu_converted_plane(uint32_t width, uint32_t height,
                                              uint32_t linesize_input,
                                              uint32_t linesize_output,
                                              const uint8_t *in, uint8_t *out)
{
    if ((width == linesize_input) && (width == linesize_output)) {
        size_t total = width * height;
        memcpy(out, in, total);
        in += total;
    } else {
        for (size_t y = 0; y < height; y++) {
            memcpy(out, in, width);
            out += linesize_output;
            in += linesize_input;
        }
    }

    return in;
}

void lite_obs_core_video::set_gpu_converted_data_internal(bool using_nv12_tex, video_frame *output, const struct video_data *input, video_format format, uint32_t width, uint32_t height)
{
    if (using_nv12_tex) {
        const uint8_t *const in_uv = set_gpu_converted_plane(
                    width, height, input->frame.linesize[0], output->linesize[0],
                input->frame.data[0], output->data[0]);

        const uint32_t height_d2 = height / 2;
        set_gpu_converted_plane(width, height_d2, input->frame.linesize[0],
                output->linesize[1], in_uv,
                output->data[1]);
    } else {
        switch (format) {
        case video_format::VIDEO_FORMAT_I420: {
            set_gpu_converted_plane(width, height,
                                    input->frame.linesize[0],
                    output->linesize[0],
                    input->frame.data[0],
                    output->data[0]);

            const uint32_t width_d2 = width / 2;
            const uint32_t height_d2 = height / 2;

            set_gpu_converted_plane(width_d2, height_d2,
                                    input->frame.linesize[1],
                    output->linesize[1],
                    input->frame.data[1],
                    output->data[1]);

            set_gpu_converted_plane(width_d2, height_d2,
                                    input->frame.linesize[2],
                    output->linesize[2],
                    input->frame.data[2],
                    output->data[2]);

            break;
        }
        case video_format::VIDEO_FORMAT_NV12: {
            set_gpu_converted_plane(width, height,
                                    input->frame.linesize[0],
                    output->linesize[0],
                    input->frame.data[0],
                    output->data[0]);

            const uint32_t height_d2 = height / 2;
            set_gpu_converted_plane(width, height_d2,
                                    input->frame.linesize[1],
                    output->linesize[1],
                    input->frame.data[1],
                    output->data[1]);

            break;
        }
        case video_format::VIDEO_FORMAT_I444: {
            set_gpu_converted_plane(width, height,
                                    input->frame.linesize[0],
                    output->linesize[0],
                    input->frame.data[0],
                    output->data[0]);

            set_gpu_converted_plane(width, height,
                                    input->frame.linesize[1],
                    output->linesize[1],
                    input->frame.data[1],
                    output->data[1]);

            set_gpu_converted_plane(width, height,
                                    input->frame.linesize[2],
                    output->linesize[2],
                    input->frame.data[2],
                    output->data[2]);

            break;
        }

        case video_format::VIDEO_FORMAT_NONE:
        case video_format::VIDEO_FORMAT_YVYU:
        case video_format::VIDEO_FORMAT_YUY2:
        case video_format::VIDEO_FORMAT_UYVY:
        case video_format::VIDEO_FORMAT_RGBA:
        case video_format::VIDEO_FORMAT_BGRA:
        case video_format::VIDEO_FORMAT_BGRX:
        case video_format::VIDEO_FORMAT_Y800:
        case video_format::VIDEO_FORMAT_BGR3:
        case video_format::VIDEO_FORMAT_I422:
        case video_format::VIDEO_FORMAT_I40A:
        case video_format::VIDEO_FORMAT_I42A:
        case video_format::VIDEO_FORMAT_YUVA:
        case video_format::VIDEO_FORMAT_AYUV:
            /* unimplemented */
            ;
        }
    }
}

void lite_obs_core_video::set_gpu_converted_data(video_frame *output,
                                                 const struct video_data *input,
                                                 const struct video_output_info *info)
{
    set_gpu_converted_data_internal(false, output, input,
                                    info->format, info->width,
                                    info->height);
}

void lite_obs_core_video::output_video_data(video_data *input_frame, int count)
{
    video_frame output_frame;
    bool locked;

    const auto info = d_ptr->video->video_output_get_info();

    locked = d_ptr->video->video_output_lock_frame(&output_frame, count, input_frame->timestamp);
    if (locked) {
        if (d_ptr->gpu_conversion) {
            set_gpu_converted_data(&output_frame, input_frame, info);
        } else {
            //todo
            //copy_rgbx_frame(&output_frame, input_frame, info);
        }

        d_ptr->video->video_output_unlock_frame();
    }
}

void lite_obs_core_video::output_frame(bool raw_active, const bool gpu_active)
{
    int cur_texture = d_ptr->cur_texture;
    int prev_texture = cur_texture == 0 ? NUM_TEXTURES - 1 : cur_texture - 1;

    video_data frame;
    bool frame_ready = 0;

    gs_enter_contex(d_ptr->graphics);

    render_video(raw_active, gpu_active, cur_texture, prev_texture);

    if (raw_active) {
        frame_ready = download_frame(prev_texture, &frame);
    }

    gs_flush();

    gs_leave_context();

    if (raw_active && frame_ready) {
        struct obs_vframe_info vframe_info;
        circlebuf_pop_front(&d_ptr->vframe_info_buffer, &vframe_info, sizeof(vframe_info));

        frame.timestamp = vframe_info.timestamp;
        output_video_data(&frame, vframe_info.count);
    }

    if (++d_ptr->cur_texture == NUM_TEXTURES)
        d_ptr->cur_texture = 0;
}

bool lite_obs_core_video::graphics_loop(obs_graphics_context *context)
{
    const bool stop_requested = d_ptr->video->video_output_stopped();

    uint64_t frame_start = os_gettime_ns();
    uint64_t frame_time_ns;
    bool raw_active = d_ptr->raw_active > 0;
    const bool gpu_active = d_ptr->gpu_encoder_active > 0;
    const bool active = raw_active || gpu_active;

    if (!context->was_active && active)
        clear_base_frame_data();
    if (!context->raw_was_active && raw_active)
        clear_raw_frame_data();
    if (!context->gpu_was_active && gpu_active)
        clear_gpu_frame_data();

    context->gpu_was_active = gpu_active;
    context->raw_was_active = raw_active;
    context->was_active = active;

    output_frame(raw_active, gpu_active);

    frame_time_ns = os_gettime_ns() - frame_start;

    video_sleep(raw_active, gpu_active, &d_ptr->video_time, context->interval);

    context->frame_time_total_ns += frame_time_ns;
    context->fps_total_ns += (d_ptr->video_time - context->last_time);
    context->fps_total_frames++;

    if (context->fps_total_ns >= 1000000000ULL) {
        d_ptr->video_fps = (double)context->fps_total_frames / ((double)context->fps_total_ns / 1000000000.0);
        d_ptr->video_avg_frame_time_ns = context->frame_time_total_ns / (uint64_t)context->fps_total_frames;

        context->frame_time_total_ns = 0;
        context->fps_total_ns = 0;
        context->fps_total_frames = 0;
    }

    return !stop_requested;
}

void lite_obs_core_video::graphics_task_func()
{
    const uint64_t interval = d_ptr->video->video_output_get_frame_time();

    d_ptr->video_time = os_gettime_ns();
    d_ptr->video_frame_interval_ns = interval;

    srand((unsigned int)time(NULL));

    obs_graphics_context context;
    context.interval = interval;
    context.frame_time_total_ns = 0;
    context.fps_total_ns = 0;
    context.fps_total_frames = 0;
    context.last_time = 0;
    context.gpu_was_active = false;
    context.raw_was_active = false;
    context.was_active = false;

    while (graphics_loop(&context))
        ;
}

std::unique_ptr<graphics_subsystem> &lite_obs_core_video::graphics()
{
    return d_ptr->graphics;
}

uint32_t lite_obs_core_video::total_frames()
{
    return d_ptr->total_frames;
}

uint32_t lite_obs_core_video::lagged_frames()
{
    return d_ptr->lagged_frames;
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
        d_ptr->conversion_techs[0] = "Convert_Planar_Y";
        d_ptr->conversion_techs[1] = "Convert_Planar_U_Left";
        d_ptr->conversion_techs[2] = "Convert_Planar_V_Left";
        d_ptr->conversion_width_i = 1.f / (float)d_ptr->output_width;
        break;
    case video_format::VIDEO_FORMAT_NV12:
        d_ptr->conversion_needed = true;
        d_ptr->conversion_techs[0] = "Convert_NV12_Y";
        d_ptr->conversion_techs[1] = "Convert_NV12_UV";
        d_ptr->conversion_width_i = 1.f / (float)d_ptr->output_width;
        break;
    case video_format::VIDEO_FORMAT_I444:
        d_ptr->conversion_needed = true;
        d_ptr->conversion_techs[0] = "Convert_Planar_Y";
        d_ptr->conversion_techs[1] = "Convert_Planar_U";
        d_ptr->conversion_techs[2] = "Convert_Planar_V";
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
    do {
        d_ptr->graphics = gs_create_graphics_system();
        if (!d_ptr->graphics) {
            break;
        }

        gs_enter_contex(d_ptr->graphics);
        if (d_ptr->ovi.gpu_conversion && !init_gpu_conversion()) {
            clear_gpu_conversion_textures();
            gs_leave_context();
            break;
        }

        if (!init_textures()) {
            gs_leave_context();
            break;
        }
        gs_leave_context();

        graphics_task_func();
    } while(false);

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

    d_ptr->graphics.reset();

    blog(LOG_DEBUG, "graphics_thread_internal stopped.");
}

void lite_obs_core_video::graphics_thread(void *param)
{
    lite_obs_core_video *p = (lite_obs_core_video *)param;
    p->graphics_thread_internal();
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

int lite_obs_core_video::lite_obs_start_video(obs_video_info *ovi)
{
    video_output_info vi;
    make_video_info(&vi, ovi);

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

    d_ptr->output_format = ovi->output_format;
    d_ptr->base_width = ovi->base_width;
    d_ptr->base_height = ovi->base_height;
    d_ptr->output_width = ovi->output_width;
    d_ptr->output_height = ovi->output_height;
    d_ptr->gpu_conversion = ovi->gpu_conversion;

    set_video_matrix(ovi);
    d_ptr->ovi = *ovi;

    gs_device::gs_check_device_context();
    d_ptr->thread_initialized = true;
    d_ptr->video_thread = std::thread(lite_obs_core_video::graphics_thread, this);
    return OBS_VIDEO_SUCCESS;
}

bool lite_obs_core_video::lite_obs_video_active()
{
    return d_ptr->raw_active > 0 || d_ptr->gpu_encoder_active > 0;
}

void lite_obs_core_video::lite_obs_stop_video()
{
    if (d_ptr->video) {
        d_ptr->video->video_output_stop();
        blog(LOG_DEBUG, "video output stopped.");
    }

    if (d_ptr->thread_initialized) {
        if (d_ptr->video_thread.joinable()) {
            d_ptr->video_thread.join();
            d_ptr->thread_initialized = false;
            blog(LOG_DEBUG, "video thread stopped");
        }
    }

    if (d_ptr->video) {
        d_ptr->video->video_output_close();
        d_ptr->video.reset();
        blog(LOG_DEBUG, "video output destroyed.");
    }
}

std::shared_ptr<video_output> lite_obs_core_video::core_video()
{
    return d_ptr->video;
}

obs_video_info *lite_obs_core_video::lite_obs_core_video_info()
{
    return &d_ptr->ovi;
}

