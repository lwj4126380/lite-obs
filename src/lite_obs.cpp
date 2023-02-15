#include "lite_obs.h"
#include <mutex>
#include <list>

#include "media-io/video_output.h"

#include "graphics/gs_subsystem.h"
#include "graphics/gs_texture.h"
#include "graphics/gs_texture_render.h"
#include "graphics/gs_program.h"
#include "graphics/gs_stagesurf.h"

#include "util/log.h"
#include "obs-defs.h"
#include "lite_obs_core_video.h"
#include "lite_obs_core_audio.h"

lite_obs obs;

struct lite_obs_data
{
    std::recursive_mutex sources_mutex;
    std::recursive_mutex audio_sources_mutex;

    std::list<std::shared_ptr<lite_source>> sources{};
    std::list<std::shared_ptr<lite_source>> audio_sources{};
};

struct lite_obs_private
{
    lite_obs_data data{};
    lite_obs_core_video video{};
    lite_obs_core_audio audio{};
};

lite_obs::lite_obs()
{
    d_ptr = std::make_unique<lite_obs_private>();
}

lite_obs::~lite_obs()
{
    obs_shutdown();
}

void lite_obs::add_source(std::shared_ptr<lite_source> source, bool is_audio_source)
{
    if (is_audio_source) {
        std::lock_guard<std::recursive_mutex> lock(d_ptr->data.audio_sources_mutex);
        d_ptr->data.audio_sources.push_back(source);
    }

    std::lock_guard<std::recursive_mutex> lock(d_ptr->data.sources_mutex);
    d_ptr->data.sources.push_back(source);
}

void lite_obs::remove_source(std::shared_ptr<lite_source> source)
{
    {
        std::lock_guard<std::recursive_mutex> lock(d_ptr->data.audio_sources_mutex);
        for (auto iter = d_ptr->data.audio_sources.begin(); iter != d_ptr->data.audio_sources.end(); iter++) {
            if (source == *iter) {
                d_ptr->data.audio_sources.erase(iter);
                break;
            }
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock(d_ptr->data.sources_mutex);
        for (auto iter = d_ptr->data.sources.begin(); iter != d_ptr->data.sources.end(); iter++) {
            if (source == *iter) {
                d_ptr->data.sources.erase(iter);
                break;
            }
        }
    }
}

#define OBS_SIZE_MIN 2
#define OBS_SIZE_MAX (32 * 1024)

static inline bool size_valid(uint32_t width, uint32_t height)
{
    return (width >= OBS_SIZE_MIN && height >= OBS_SIZE_MIN &&
        width <= OBS_SIZE_MAX && height <= OBS_SIZE_MAX);
}

int lite_obs::obs_reset_video(obs_video_info *ovi)
{
    if (d_ptr->video.lite_obs_video_active())
        return OBS_VIDEO_CURRENTLY_ACTIVE;

    if (!size_valid(ovi->output_width, ovi->output_height) ||
            !size_valid(ovi->base_width, ovi->base_height))
            return OBS_VIDEO_INVALID_PARAM;

    if (ovi->output_format != video_format::VIDEO_FORMAT_NV12
            && ovi->output_format != video_format::VIDEO_FORMAT_I420
            && ovi->output_format != video_format::VIDEO_FORMAT_I444) {
        blog(LOG_DEBUG, "video output format request one of (NV12 I420 I444)");
        return OBS_VIDEO_INVALID_PARAM;
    }

    d_ptr->video.lite_obs_stop_video();

    ovi->output_width &= 0xFFFFFFFC;
    ovi->output_height &= 0xFFFFFFFE;

    return d_ptr->video.lite_obs_start_video(ovi);
}

bool lite_obs::obs_reset_audio(const obs_audio_info *oai)
{
    return d_ptr->audio.lite_obs_start_audio(oai);
}

void lite_obs::obs_enter_graphics_context()
{
    gs_enter_contex(d_ptr->video.graphics());
}

void lite_obs::obs_leave_graphics_context()
{
    gs_leave_context();
}

lite_obs_core_video *lite_obs::obs_core_video()
{
    return &d_ptr->video;
}

lite_obs_core_audio *lite_obs::obs_core_audio()
{
    return &d_ptr->audio;
}

void lite_obs::obs_shutdown()
{
    d_ptr->video.lite_obs_stop_video();
    d_ptr->audio.lite_obs_stop_audio();
//    stop_audio();

//    obs_free_audio();
//    obs_free_data();

//    obs_rtc_capture_free(true);
//    obs_free_hotkeys();

//    proc_handler_destroy(obs->procs);
//    signal_handler_destroy(obs->signals);
//    obs->procs = NULL;
    //    obs->signals = NULL;
}
