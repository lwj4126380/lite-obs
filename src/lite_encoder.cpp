#include "lite_encoder.h"

#include "media-io/video_output.h"
#include "media-io/audio_output.h"
#include "util/circlebuf.h"
#include "util/log.h"
#include <mutex>
#include <atomic>
#include <list>
#include <vector>

class lite_obs_output;

struct encoder_callback {
    bool sent_first_packet{};
    new_packet cb;
    void *param{};
};

struct lite_obs_encoder_private
{
    std::recursive_mutex init_mutex;

    uint32_t samplerate{};
    size_t planes{};
    size_t blocksize{};
    size_t framesize{};
    size_t framesize_bytes{};

    size_t mixer_idx{};

    uint32_t scaled_width{};
    uint32_t scaled_height{};
    video_format preferred_format{};

    std::atomic_bool active{};
    std::atomic_bool paused{};
    bool initialized{};

    uint32_t timebase_num{};
    uint32_t timebase_den{};

    int64_t cur_pts{};

    circlebuf audio_input_buffer[MAX_AV_PLANES]{};
    uint8_t *audio_output_buffer[MAX_AV_PLANES]{};

    /* if a video encoder is paired with an audio encoder, make it start
         * up at the specific timestamp.  if this is the audio encoder,
         * wait_for_video makes it wait until it's ready to sync up with
         * video */
    bool wait_for_video{};
    bool first_received{};
    std::weak_ptr<lite_obs_encoder> paired_encoder{};
    int64_t offset_usec{};
    uint64_t first_raw_ts{};
    uint64_t start_ts{};

    std::mutex outputs_mutexoutputs_mutex;
    std::list<std::weak_ptr<lite_obs_output>> outputs{};

    bool destroy_on_stop{};

    /* stores the video/audio media output pointer.  video_t *or audio_t **/
    std::weak_ptr<video_output> v_media{};
    std::weak_ptr<audio_output> a_media{};

    std::recursive_mutex callbacks_mutexoutputs_mutex;
    std::list<encoder_callback> callbacks{};

    uint32_t sei_rate{};
    std::mutex sei_mutex;
    std::vector<uint8_t> custom_sei{};
    size_t custom_sei_size{};
    uint64_t sei_counting{};

    lite_obs_encoder_private() {
        custom_sei.resize(sizeof(uint8_t) * 1024 * 100);
    }
};

lite_obs_encoder::lite_obs_encoder(size_t mixer_idx)
{
    d_ptr = std::make_unique<lite_obs_encoder_private>();
    d_ptr->mixer_idx = mixer_idx;
}

lite_obs_encoder::~lite_obs_encoder()
{

}

void lite_obs_encoder::lite_obs_encoder_set_scaled_size(uint32_t width, uint32_t height)
{
    if (lite_obs_encoder_get_encoder_type() != obs_encoder_type::OBS_ENCODER_VIDEO)
        return;

    if (d_ptr->active) {
        blog(LOG_WARNING, "encoder Cannot set the scaled resolution while the encoder is active");
        return;
    }

    d_ptr->scaled_width = width;
    d_ptr->scaled_height = height;
}

bool lite_obs_encoder::lite_obs_encoder_scaling_enabled()
{
    return d_ptr->scaled_width || d_ptr->scaled_height;
}

uint32_t lite_obs_encoder::lite_obs_encoder_get_width()
{
    if (lite_obs_encoder_get_encoder_type() != obs_encoder_type::OBS_ENCODER_VIDEO) {
        blog(LOG_WARNING, "obs_encoder_get_width:  encoder is not a video encoder");
        return 0;
    }

    auto vo = d_ptr->v_media.lock();
    if (!vo)
        return 0;

    return d_ptr->scaled_width != 0 ? d_ptr->scaled_width : vo->video_output_get_width();
}

uint32_t lite_obs_encoder::lite_obs_encoder_get_height()
{
    if (lite_obs_encoder_get_encoder_type() != obs_encoder_type::OBS_ENCODER_VIDEO) {
        blog(LOG_WARNING, "obs_encoder_get_height:  encoder is not a video encoder");
        return 0;
    }

    auto vo = d_ptr->v_media.lock();
    if (!vo)
        return 0;

    return d_ptr->scaled_height != 0 ? d_ptr->scaled_height : vo->video_output_get_height();
}

uint32_t lite_obs_encoder::lite_obs_encoder_get_sample_rate()
{
    if (lite_obs_encoder_get_encoder_type() != obs_encoder_type::OBS_ENCODER_AUDIO) {
        blog(LOG_WARNING, "lite_obs_encoder_get_sample_rate: encoder is not a audio encoder");
        return 0;
    }

    auto ao = d_ptr->a_media.lock();
    if (!ao)
        return 0;

    return d_ptr->samplerate != 0 ? d_ptr->samplerate : ao->audio_output_get_sample_rate();
}

size_t lite_obs_encoder::lite_obs_encoder_get_frame_size()
{
    if (lite_obs_encoder_get_encoder_type() != obs_encoder_type::OBS_ENCODER_AUDIO) {
        blog(LOG_WARNING, "lite_obs_encoder_get_frame_size: encoder is not a audio encoder");
        return 0;
    }

    return d_ptr->framesize;
}

void lite_obs_encoder::lite_obs_encoder_set_preferred_video_format(video_format format)
{
    if (lite_obs_encoder_get_encoder_type() != obs_encoder_type::OBS_ENCODER_VIDEO)
        return;

    d_ptr->preferred_format = format;
}

video_format lite_obs_encoder::lite_obs_encoder_get_preferred_video_format()
{
    if (lite_obs_encoder_get_encoder_type() != obs_encoder_type::OBS_ENCODER_VIDEO)
        return video_format::VIDEO_FORMAT_NONE;

    return d_ptr->preferred_format;
}

bool lite_obs_encoder::lite_obs_encoder_get_extra_data(uint8_t **extra_data, size_t *size)
{


    return true;
}

void lite_obs_encoder::lite_obs_encoder_set_video(std::shared_ptr<video_output> video)
{
    if (lite_obs_encoder_get_encoder_type() != obs_encoder_type::OBS_ENCODER_VIDEO) {
        blog(LOG_WARNING, "obs_encoder_set_video: encoder is not a video encoder");
        return;
    }

    if (!video)
        return;

    auto voi = video->video_output_get_info();

    d_ptr->v_media = video;
    d_ptr->timebase_num = voi->fps_den;
    d_ptr->timebase_den = voi->fps_num;
}

void lite_obs_encoder::lite_obs_encoder_set_audio(std::shared_ptr<audio_output> audio)
{
    if (lite_obs_encoder_get_encoder_type() != obs_encoder_type::OBS_ENCODER_AUDIO) {
        blog(LOG_WARNING, "lite_obs_encoder_set_audio: encoder is not a audio encoder");
        return;
    }

    if (!audio)
        return;

    d_ptr->a_media = audio;
    d_ptr->timebase_num = 1;
    d_ptr->timebase_den = audio->audio_output_get_sample_rate();
}

std::shared_ptr<video_output> lite_obs_encoder::lite_obs_encoder_video()
{
    if (lite_obs_encoder_get_encoder_type() != obs_encoder_type::OBS_ENCODER_VIDEO) {
        blog(LOG_WARNING, "lite_obs_encoder_video: encoder is not a video encoder");
        return nullptr;
    }

    return d_ptr->v_media.lock();
}

std::shared_ptr<audio_output> lite_obs_encoder::lite_obs_encoder_audio()
{
    if (lite_obs_encoder_get_encoder_type() != obs_encoder_type::OBS_ENCODER_AUDIO  ) {
        blog(LOG_WARNING, "lite_obs_encoder_audio: encoder is not a audio encoder");
        return nullptr;
    }

    return d_ptr->a_media.lock();
}

bool lite_obs_encoder::lite_obs_encoder_active()
{
    return d_ptr->active;
}

void *lite_obs_encoder::lite_obs_encoder_get_type_data()
{

}

uint32_t lite_obs_encoder::lite_obs_encoder_get_caps()
{

}

void lite_obs_encoder::lite_obs_encoder_set_sei(char *sei, int len)
{

}

void lite_obs_encoder::lite_obs_encoder_clear_sei()
{

}

bool lite_obs_encoder::lite_obs_encoder_get_sei(uint8_t *sei, int *sei_len)
{

}

