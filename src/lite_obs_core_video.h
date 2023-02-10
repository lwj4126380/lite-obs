#pragma once

#include <memory>
#include "lite_obs.h"

struct lite_obs_core_video_private;
struct obs_graphics_context;
class gs_texture;
class gs_program;
class graphics_subsystem;
class lite_obs_core_video
{
public:
    lite_obs_core_video();
    ~lite_obs_core_video();

    int lite_obs_start_video(obs_video_info *ovi);

    bool lite_obs_video_active();
    void lite_obs_stop_video();

    static void graphics_thread(void *param);

    std::unique_ptr<graphics_subsystem> &graphics();

private:
    void set_video_matrix(obs_video_info *ovi);
    void calc_gpu_conversion_sizes();

    void clear_gpu_conversion_textures();
    bool init_gpu_conversion();
    void clear_gpu_copy_surface();
    bool init_gpu_copy_surface(size_t i);
    bool init_textures();

    void graphics_task_func();

    void clear_base_frame_data(void);
    void clear_raw_frame_data(void);
    void clear_gpu_frame_data(void);

    void video_sleep(bool raw_active, const bool gpu_active, uint64_t *p_time, uint64_t interval_ns);
    bool resolution_close(uint32_t width, uint32_t height);
    std::shared_ptr<gs_program> get_scale_effect_internal();
    std::shared_ptr<gs_program> get_scale_effect(uint32_t width, uint32_t height);
    void stage_output_texture(int cur_texture);
    void render_convert_plane(std::shared_ptr<gs_texture> target);
    void render_convert_texture(std::shared_ptr<gs_texture> texture);
    void render_all_sources();
    void render_main_texture();
    std::shared_ptr<gs_texture> render_output_texture();
    void render_video(bool raw_active, const bool gpu_active, int cur_texture, int prev_texture);
    bool download_frame(int prev_texture, struct video_data *frame);
    void output_video_data(video_data *input_frame, int count);
    void output_frame(bool raw_active, const bool gpu_active);
    bool graphics_loop(obs_graphics_context *context);
    void graphics_thread_internal();

private:
    std::unique_ptr<lite_obs_core_video_private> d_ptr{};
};
