#pragma once

#include <memory>
#include <string>
#include "media-io/audio_output.h"

enum class obs_source_type {
    OBS_SOURCE_TYPE_INPUT,
    OBS_SOURCE_TYPE_FILTER,
    OBS_SOURCE_TYPE_TRANSITION,
    OBS_SOURCE_TYPE_SCENE,
};

/**
 * @name Source output flags
 *
 * These flags determine what type of data the source outputs and expects.
 * @{
 */

/**
 * Source has video.
 *
 * Unless SOURCE_ASYNC_VIDEO is specified, the source must include the
 * video_render callback in the source definition structure.
 */
#define OBS_SOURCE_VIDEO (1 << 0)

/**
 * Source has audio.
 *
 * Use the obs_source_output_audio function to pass raw audio data, which will
 * be automatically converted and uploaded.  If used with SOURCE_ASYNC_VIDEO,
 * audio will automatically be synced up to the video output.
 */
#define OBS_SOURCE_AUDIO (1 << 1)

/** Async video flag (use OBS_SOURCE_ASYNC_VIDEO) */
#define OBS_SOURCE_ASYNC (1 << 2)

/**
 * Source passes raw video data via RAM.
 *
 * Use the obs_source_output_video function to pass raw video data, which will
 * be automatically uploaded at the specified timestamp.
 *
 * If this flag is specified, it is not necessary to include the video_render
 * callback.  However, if you wish to use that function as well, you must call
 * obs_source_getframe to get the current frame data, and
 * obs_source_releaseframe to release the data when complete.
 */
#define OBS_SOURCE_ASYNC_VIDEO (OBS_SOURCE_ASYNC | OBS_SOURCE_VIDEO)

/**
 * Source uses custom drawing, rather than a default effect.
 *
 * If this flag is specified, the video_render callback will pass a NULL
 * effect, and effect-based filters will not use direct rendering.
 */
#define OBS_SOURCE_CUSTOM_DRAW (1 << 3)


class lite_source;
class lite_source_impl
{
public:
    std::string id;

    obs_source_type type;

    uint32_t output_flags;

    uint32_t get_width()
    {
        return 0;
    }

    uint32_t get_height()
    {
        return 0;
    }


    void activate(){}
    void deactivate(){}

    void show(){}
    void hide(){}

    void video_tick(float seconds){}
    void video_render(){}

    bool audio_render(uint64_t *ts_out, struct obs_source_audio_mix *audio_output, uint32_t mixers, size_t channels,size_t sample_rate)
    {
        return true;
    }

    bool audio_mix(uint64_t *ts_out, struct audio_output_data *audio_output, size_t channels, size_t sample_rate)
    {
        return true;
    }

};

struct lite_source_private;
class lite_source : public std::enable_shared_from_this<lite_source>
{
public:
    lite_source(const std::string &id);
    ~lite_source();

    bool is_audio_source();

private:
    void allocate_audio_output_buffer();

private:
    std::unique_ptr<lite_source_private> d_ptr{};
};

typedef void (*obs_source_audio_capture)(void *param, std::shared_ptr<lite_source> source, const audio_data *audio_data, bool muted);

std::shared_ptr<lite_source> obs_source_create(const std::string &id);
void obs_source_destroy(std::shared_ptr<lite_source> source);
