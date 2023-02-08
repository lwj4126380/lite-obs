#ifndef LITE_OBS_H
#define LITE_OBS_H

#include "lite_obs_info.h"
#include <memory>



struct lite_obs_data;
struct lite_obs_private;

class lite_source;
class lite_obs
{
public:
    lite_obs();

    void add_source(std::shared_ptr<lite_source> source, bool is_audio_source);
    void remove_source(std::shared_ptr<lite_source> source);

    //thread request
    int obs_reset_video(obs_video_info *ovi);

    void obs_shutdown();

private:
    int obs_init_graphics(obs_video_info *ovi);

private:
    std::unique_ptr<lite_obs_private> d_ptr{};
};

extern lite_obs obs;

#endif // LITE_OBS_H
