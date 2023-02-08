#pragma once

#include <memory>
#include "lite_obs.h"

struct lite_obs_core_video_private;
struct obs_graphics_context;
class lite_obs_core_video
{
public:
    lite_obs_core_video();
    ~lite_obs_core_video();

    int lite_obs_init_graphics();
    void lite_obs_free_graphics();

    int lite_obs_core_video_init(obs_video_info *ovi);

    bool lite_obs_video_active();
    void lite_obs_stop_video();

    void lite_obs_video_free_data();

    static void graphics_thread(void *param);

private:
    void set_video_matrix(obs_video_info *ovi);
    void calc_gpu_conversion_sizes();

    void clear_gpu_conversion_textures();
    bool init_gpu_conversion();
    void clear_gpu_copy_surface();
    bool init_gpu_copy_surface(size_t i);
    bool init_textures();

    void clear_base_frame_data(void);
    void clear_raw_frame_data(void);
    void clear_gpu_frame_data(void);

    void video_sleep(bool raw_active, const bool gpu_active, uint64_t *p_time, uint64_t interval_ns);
    void render_video(bool raw_active, const bool gpu_active, int cur_texture, int prev_texture);
    bool download_frame(int prev_texture, struct video_data *frame);
    void output_video_data(video_data *input_frame, int count);
    void output_frame(bool raw_active, const bool gpu_active);
    bool graphics_loop(obs_graphics_context *context);
    void graphics_thread_internal();

private:
    std::unique_ptr<lite_obs_core_video_private> d_ptr{};
};
