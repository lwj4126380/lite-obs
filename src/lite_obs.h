#ifndef LITE_OBS_H
#define LITE_OBS_H

#include "lite_obs_info.h"
#include <memory>



struct lite_obs_data;
struct lite_obs_private;

class lite_source;
class video_output;
class audio_output;
class lite_obs_core_video;
class lite_obs_core_audio;
class lite_obs
{
public:
    lite_obs();
    ~lite_obs();

    void add_source(std::shared_ptr<lite_source> source, bool is_audio_source);
    void remove_source(std::shared_ptr<lite_source> source);

    int obs_reset_video(obs_video_info *ovi);
    bool obs_reset_audio(const obs_audio_info *oai);

    void obs_enter_graphics_context();
    void obs_leave_graphics_context();

    lite_obs_core_video *obs_core_video();
    lite_obs_core_audio *obs_core_audio();

    void obs_shutdown();

private:
    std::unique_ptr<lite_obs_private> d_ptr{};
};

extern lite_obs obs;

#endif // LITE_OBS_H
