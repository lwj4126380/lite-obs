#pragma once

#include "lite_encoder.h"

struct lite_ffmpeg_video_encoder_private;
class lite_ffmpeg_video_encoder : public lite_obs_encoder
{
public:
    lite_ffmpeg_video_encoder(size_t mixer_idx);
    ~lite_ffmpeg_video_encoder();
    virtual const char *i_encoder_codec();
    virtual obs_encoder_type i_encoder_type();
    virtual bool i_create();
    virtual void i_destroy();
    virtual bool encoder_valid();
    virtual bool i_encode(encoder_frame *frame, std::shared_ptr<encoder_packet> packet, bool *received_packet);
    virtual bool i_get_extra_data(uint8_t **extra_data, size_t *size);
    virtual bool i_get_sei_data(uint8_t **sei_data, size_t *size);
    virtual void i_get_audio_info(struct audio_convert_info *info);
    virtual void i_get_video_info(struct video_scale_info *info);
    virtual bool i_gpu_encode_available();

private:
    bool update_settings();
    bool init_codec();

private:
    std::unique_ptr<lite_ffmpeg_video_encoder_private> d_ptr{};
};
