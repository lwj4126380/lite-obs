#pragma once

#include <stdint.h>
#include "media-io/video_info.h"
#include "media-io/media-io-defs.h"

#define NUM_TEXTURES 2
#define NUM_CHANNELS 3

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
    video_format output_format{}; /**< Output format */

    /** Use shaders to convert to different color formats */
    bool gpu_conversion{};

    video_colorspace colorspace{}; /**< YUV type (if YUV) */
    video_range_type range{};      /**< YUV range (if YUV) */

    obs_scale_type scale_type{}; /**< How to scale if scaling */
};
