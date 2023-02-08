#include "lite_obs_core_video.h"
#include "graphics/gs_subsystem.h"
#include <atomic>
struct lite_obs_core_video_private
{
    std::unique_ptr<graphics_subsystem> graphics{};

    std::atomic_long raw_active{};
    std::atomic_long gpu_encoder_active{};
};

lite_obs_core_video::lite_obs_core_video()
{
    d_ptr = std::make_unique<lite_obs_core_video_private>();
}

lite_obs_core_video::~lite_obs_core_video()
{

}

bool lite_obs_core_video::obs_video_active()
{
    return d_ptr->raw_active > 0 || d_ptr->gpu_encoder_active > 0;
}

void lite_obs_core_video::stop_video()
{

}
