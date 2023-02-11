#pragma once

#include <memory>
#include "lite_encoder_info.h"
#include "media-io/video_info.h"

struct lite_obs_encoder_private;

typedef void (*new_packet)(void *param, struct encoder_packet *packet);

class video_output;
class audio_output;
class lite_obs_encoder
{
public:
    lite_obs_encoder(size_t mixer_idx);
    ~lite_obs_encoder();

    virtual const char *lite_obs_encoder_get_encoder_codec() = 0;
    virtual obs_encoder_type lite_obs_encoder_get_encoder_type() = 0;;

    void lite_obs_encoder_set_scaled_size(uint32_t width, uint32_t height);
    bool lite_obs_encoder_scaling_enabled();

    uint32_t lite_obs_encoder_get_width();
    uint32_t lite_obs_encoder_get_height();
    uint32_t lite_obs_encoder_get_sample_rate();
    size_t lite_obs_encoder_get_frame_size();

    void lite_obs_encoder_set_preferred_video_format(video_format format);
    video_format lite_obs_encoder_get_preferred_video_format();

    bool lite_obs_encoder_get_extra_data(uint8_t **extra_data, size_t *size);

    void lite_obs_encoder_set_video(std::shared_ptr<video_output> video);
    void lite_obs_encoder_set_audio(std::shared_ptr<audio_output> audio);

    std::shared_ptr<video_output> lite_obs_encoder_video();
    std::shared_ptr<audio_output> lite_obs_encoder_audio();

    bool lite_obs_encoder_active();

    void *lite_obs_encoder_get_type_data();

    uint32_t lite_obs_encoder_get_caps();

    void lite_obs_encoder_set_sei(char *sei, int len);
    void lite_obs_encoder_clear_sei();
    bool lite_obs_encoder_get_sei(uint8_t *sei, int *sei_len);

private:
    std::unique_ptr<lite_obs_encoder_private> d_ptr{};
};
