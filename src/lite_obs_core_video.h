#pragma once

#include <memory>
#include "lite_obs.h"

struct lite_obs_core_video_private;
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

    void graphics_thread_internal();

private:
    std::unique_ptr<lite_obs_core_video_private> d_ptr{};
};
