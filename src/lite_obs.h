#ifndef LITE_OBS_H
#define LITE_OBS_H

#include "audio_output.h"
#include "video_output.h"
#include "video_scaler.h"

#include "lite_source.h"

#include "lite-obs2_global.h"

/**
 * Sent to source filters via the filter_audio callback to allow filtering of
 * audio data
 */
struct obs_audio_data {
    uint8_t *data[MAX_AV_PLANES]{};
    uint32_t frames{};
    uint64_t timestamp{};
};

enum class obs_scale_type {
    OBS_SCALE_DISABLE,
    OBS_SCALE_POINT,
    OBS_SCALE_BICUBIC,
    OBS_SCALE_BILINEAR,
    OBS_SCALE_LANCZOS,
    OBS_SCALE_AREA,
};

struct obs_video_info {
    uint32_t fps_num{}; /**< Output FPS numerator */
    uint32_t fps_den{}; /**< Output FPS denominator */

    uint32_t base_width{};  /**< Base compositing width */
    uint32_t base_height{}; /**< Base compositing height */

    uint32_t output_width{};           /**< Output width */
    uint32_t output_height{};          /**< Output height */
    enum video_format output_format{}; /**< Output format */

    /** Use shaders to convert to different color formats */
    bool gpu_conversion{};

    video_colorspace colorspace{}; /**< YUV type (if YUV) */
    video_range_type range{};      /**< YUV range (if YUV) */

    obs_scale_type scale_type{}; /**< How to scale if scaling */
};

struct lite_obs_data;
struct lite_obs_private;
class lite_obs
{
public:
    lite_obs();

    void add_source(std::shared_ptr<lite_source> source, bool is_audio_source);
    void remove_source(std::shared_ptr<lite_source> source);

    //thread request
    int obs_reset_video(obs_video_info *ovi);

private:
    int obs_init_graphics(obs_video_info *ovi);

private:
    std::unique_ptr<lite_obs_private> d_ptr{};
};

extern lite_obs obs;

#endif // LITE_OBS_H
