#pragma once

#include <memory>

struct lite_obs_core_video_private;
class lite_obs_core_video
{
public:
    lite_obs_core_video();
    ~lite_obs_core_video();

    bool obs_video_active();
    void stop_video();

private:
    std::unique_ptr<lite_obs_core_video_private> d_ptr{};
};
