#pragma once

#include "lite_output.h"

struct lite_ffmpeg_mux_private;
class lite_ffmpeg_mux : public lite_obs_output
{
public:
    lite_ffmpeg_mux();
    ~lite_ffmpeg_mux();

private:
    std::unique_ptr<lite_ffmpeg_mux_private> d_ptr{};
};
