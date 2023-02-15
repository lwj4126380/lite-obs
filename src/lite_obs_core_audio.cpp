#include "lite_obs_core_audio.h"
#include "lite_obs_info.h"
#include "media-io/audio_output.h"
#include "util/circlebuf.h"
#include "util/log.h"

struct ts_info {
    uint64_t start;
    uint64_t end;
};

#define DEBUG_AUDIO 0
#define MAX_BUFFERING_TICKS 45

struct lite_obs_core_audio_private
{
    std::shared_ptr<audio_output> audio{};

    uint64_t buffered_ts{};
    circlebuf buffered_timestamps{};
    uint64_t buffering_wait_ticks{};
    int total_buffering_ticks{};
};

lite_obs_core_audio::lite_obs_core_audio()
{
    d_ptr = std::make_unique<lite_obs_core_audio_private>();
}

lite_obs_core_audio::~lite_obs_core_audio()
{

}

std::shared_ptr<audio_output> lite_obs_core_audio::core_audio()
{
    return d_ptr->audio;
}

bool lite_obs_core_audio::audio_callback_internal(uint64_t start_ts_in, uint64_t end_ts_in,
                                                  uint64_t *out_ts, uint32_t mixers,
                                                  audio_output_data *mixes)
{
    size_t sample_rate = d_ptr->audio->audio_output_get_sample_rate();
    size_t channels = d_ptr->audio->audio_output_get_channels();
    ts_info ts = {start_ts_in, end_ts_in};
    size_t audio_size;
    uint64_t min_ts;

    circlebuf_push_back(&d_ptr->buffered_timestamps, &ts, sizeof(ts));
    circlebuf_peek_front(&d_ptr->buffered_timestamps, &ts, sizeof(ts));
    min_ts = ts.start;

    audio_size = AUDIO_OUTPUT_FRAMES * sizeof(float);

//    /* ------------------------------------------------ */
//    /* render audio data */
//    for (size_t i = 0; i < audio->render_order.num; i++) {
//        obs_source_t *source = audio->render_order.array[i];
//        obs_source_audio_render(source, mixers, channels, sample_rate,
//                                audio_size);
//    }

//    /* ------------------------------------------------ */
//    /* get minimum audio timestamp */
//    pthread_mutex_lock(&data->audio_sources_mutex);
//    const char *buffering_name = calc_min_ts(data, sample_rate, &min_ts);
//    pthread_mutex_unlock(&data->audio_sources_mutex);

//    /* ------------------------------------------------ */
//    /* if a source has gone backward in time, buffer */
//    if (min_ts < ts.start)
//        add_audio_buffering(audio, sample_rate, &ts, min_ts,
//                            buffering_name);

//    /* ------------------------------------------------ */
//    /* mix audio */
//    if (!audio->buffering_wait_ticks) {
//        for (size_t i = 0; i < audio->root_nodes.num; i++) {
//            obs_source_t *source = audio->root_nodes.array[i];

//            if (source->audio_pending)
//                continue;

//            pthread_mutex_lock(&source->audio_buf_mutex);

//            if (source->audio_output_buf[0][0] && source->audio_ts)
//                mix_audio(mixes, source, channels, sample_rate,
//                          &ts);

//            pthread_mutex_unlock(&source->audio_buf_mutex);
//        }
//    }

//    /* ------------------------------------------------ */
//    /* discard audio */
//    pthread_mutex_lock(&data->audio_sources_mutex);

//    source = data->first_audio_source;
//    while (source) {
//        pthread_mutex_lock(&source->audio_buf_mutex);
//        discard_audio(audio, source, channels, sample_rate, &ts);
//        pthread_mutex_unlock(&source->audio_buf_mutex);

//        source = (struct obs_source *)source->next_audio_source;
//    }

//    pthread_mutex_unlock(&data->audio_sources_mutex);

//    /* ------------------------------------------------ */
//    /* release audio sources */
//    release_audio_sources(audio);

    circlebuf_pop_front(&d_ptr->buffered_timestamps, NULL, sizeof(ts));

    *out_ts = ts.start;

    if (d_ptr->buffering_wait_ticks) {
        d_ptr->buffering_wait_ticks--;
        return false;
    }

    return true;
}


bool lite_obs_core_audio::audio_callback(void *param, uint64_t start_ts_in, uint64_t end_ts_in,
                                         uint64_t *out_ts, uint32_t mixers,
                                         audio_output_data *mixes)
{
    auto core_audio = (lite_obs_core_audio *)param;
    return core_audio->audio_callback_internal(start_ts_in, end_ts_in, out_ts, mixers, mixes);
}

bool lite_obs_core_audio::lite_obs_start_audio(const obs_audio_info *oai)
{
    if (d_ptr->audio && d_ptr->audio->audio_output_active())
        return false;

    free_audio();

    if (!oai)
        return true;

    audio_output_info ai{};
    ai.name = "Audio";
    ai.samples_per_sec = oai->samples_per_sec;
    ai.format = audio_format::AUDIO_FORMAT_FLOAT_PLANAR;
    ai.speakers = oai->speakers;
    ai.input_callback = lite_obs_core_audio::audio_callback;
    ai.input_param = this;

    blog(LOG_INFO, "---------------------------------");
    blog(LOG_INFO,
         "audio settings reset:\n"
         "\tsamples per sec: %d\n"
         "\tspeakers:        %d",
         (int)ai.samples_per_sec, (int)ai.speakers);

    auto audio = std::make_shared<audio_output>();
    if (audio->audio_output_open(&ai) != AUDIO_OUTPUT_SUCCESS)
        return false;

    d_ptr->audio = audio;
    return true;
}

void lite_obs_core_audio::lite_obs_stop_audio()
{
    if (d_ptr->audio) {
        d_ptr->audio->audio_output_close();
        d_ptr->audio.reset();
    }

    free_audio();
}

void lite_obs_core_audio::free_audio()
{
    if (d_ptr->audio)
        d_ptr->audio->audio_output_close();

    circlebuf_free(&d_ptr->buffered_timestamps);
    d_ptr->buffered_ts = 0;
    d_ptr->buffering_wait_ticks = 0;
    d_ptr->total_buffering_ticks = 0;
}
