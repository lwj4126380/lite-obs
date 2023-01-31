#include "lite_source.h"
#include "util/circlebuf.h"
#include "util/log.h"
#include "audio_info.h"
#include "video_info.h"
#include "lite_obs.h"
#include "audio_resampler.h"
#include <mutex>
#include <list>

struct audio_cb_info {
    obs_source_audio_capture callback{};
    void *param{};
};

struct lite_source_private
{
    std::unique_ptr<lite_source_impl> impl{};
    /* general exposed flags that can be set for the source */
    uint32_t flags{};
    uint32_t default_flags{};

    /* signals to call the source update in the video thread */
    bool defer_update{};

    /* ensures show/hide are only called once */
    volatile long show_refs{};

    /* ensures activate/deactivate are only called once */
    volatile long activate_refs{};

    /* used to indicate that the source has been removed and all
         * references to it should be released (not exactly how I would prefer
         * to handle things but it's the best option) */
    bool removed{};

    bool active{};
    bool showing{};

    /* used to temporarily disable sources if needed */
    bool enabled{};

    /* timing (if video is present, is based upon video) */
    volatile bool timing_set{};
    volatile uint64_t timing_adjust{};
    uint64_t resample_offset{};
    uint64_t last_audio_ts{};
    uint64_t next_audio_ts_min{};
    uint64_t next_audio_sys_ts_min{};
    uint64_t last_frame_ts{};
    uint64_t last_sys_timestamp{};
    bool async_rendered{};

    /* audio */
    bool audio_failed{};
    bool audio_pending{};
    bool pending_stop{};
    bool audio_active{};
    bool user_muted{};
    bool muted{};
    lite_source *next_audio_source{};
    lite_source **prev_next_audio_source{};
    uint64_t audio_ts{};
    circlebuf audio_input_buf[MAX_AUDIO_CHANNELS]{};
    size_t last_audio_input_buf_size{};
    float *audio_output_buf[MAX_AUDIO_MIXES][MAX_AUDIO_CHANNELS]{};
    resample_info sample_info{};
    std::shared_ptr<audio_resampler> resampler{};
    std::mutex audio_buf_mutex;
    std::mutex audio_mutex;
    std::mutex audio_cb_mutex;
    std::list<audio_cb_info> audio_cb_list{};
    obs_audio_data audio_data{};
    size_t audio_storage_size{};
    uint32_t audio_mixers{};
    float user_volume{};
    float volume{};
    int64_t sync_offset{};
    int64_t last_sync_offset{};
    float balance{};

    /* async video data */
    //    gs_texture_t *async_textures[MAX_AV_PLANES];
    //    gs_texrender_t *async_texrender;
    //    struct obs_source_frame *cur_async_frame;
    //    bool async_gpu_conversion;
    //    enum video_format async_format;
    //    bool async_full_range;
    //    enum video_format async_cache_format;
    //    bool async_cache_full_range;
    //    enum gs_color_format async_texture_formats[MAX_AV_PLANES];
    //    int async_channel_count;
    //    bool async_flip;
    //    bool async_flip_h;
    //    bool async_active;
    //    bool async_update_texture;
    //    bool async_unbuffered;
    //    bool async_decoupled;
    //    struct obs_source_frame *async_preload_frame;
    //    DARRAY(struct async_frame) async_cache;
    //    DARRAY(struct obs_source_frame *) async_frames;
    std::mutex async_mutex;
    //    uint32_t async_width;
    //    uint32_t async_height;
    //    uint32_t async_cache_width;
    //    uint32_t async_cache_height;
    //    uint32_t async_convert_width[MAX_AV_PLANES];
    //    uint32_t async_convert_height[MAX_AV_PLANES];
};


std::shared_ptr<lite_source> obs_source_create(const std::string &id)
{
    auto source = std::make_shared<lite_source>(id);
    obs.add_source(source, source->is_audio_source());
    return source;
}

void obs_source_destroy(std::shared_ptr<lite_source> source)
{
    obs.remove_source(source);
}

lite_source::lite_source(const std::string &id)
{
    d_ptr = std::make_unique<lite_source_private>();
    d_ptr->impl = std::make_unique<lite_source_impl>();

    d_ptr->user_volume = 1.0f;
    d_ptr->volume = 1.0f;
    d_ptr->sync_offset = 0;
    d_ptr->balance = 0.5f;
    d_ptr->audio_active = true;


    if (is_audio_source())
        allocate_audio_output_buffer();

    d_ptr->audio_mixers = 0xFF;
    d_ptr->enabled = true;
}

lite_source::~lite_source()
{
    blog(LOG_DEBUG, "%ssource '%s' destroyed", d_ptr->impl->id.c_str());

//    for (i = 0; i < source->async_cache.num; i++)
//        obs_source_frame_decref(source->async_cache.array[i].frame);

//    gs_enter_context(obs->video.graphics);
//    if (source->async_texrender)
//        gs_texrender_destroy(source->async_texrender);
//    if (source->async_prev_texrender)
//        gs_texrender_destroy(source->async_prev_texrender);
//    for (size_t c = 0; c < MAX_AV_PLANES; c++) {
//        gs_texture_destroy(source->async_textures[c]);
//        gs_texture_destroy(source->async_prev_textures[c]);
//    }
//    if (source->filter_texrender)
//        gs_texrender_destroy(source->filter_texrender);
//    if (source->async_holder_image)
//        gs_texture_destroy(source->async_holder_image);
//    gs_leave_context();

//    for (i = 0; i < MAX_AV_PLANES; i++)
//        bfree(source->audio_data.data[i]);
//    for (i = 0; i < MAX_AUDIO_CHANNELS; i++)
//        circlebuf_free(&source->audio_input_buf[i]);
//    audio_resampler_destroy(source->resampler);
//    bfree(source->audio_output_buf[0][0]);
//    bfree(source->audio_mix_buf[0]);

//    obs_source_frame_destroy(source->async_preload_frame);

//    if (source->info.type == OBS_SOURCE_TYPE_TRANSITION)
//        obs_transition_free(source);

//    da_free(source->audio_actions);
//    da_free(source->audio_cb_list);
//    da_free(source->async_cache);
//    da_free(source->async_frames);
//    da_free(source->filters);
//    pthread_mutex_destroy(&source->filter_mutex);
//    pthread_mutex_destroy(&source->audio_actions_mutex);
//    pthread_mutex_destroy(&source->audio_buf_mutex);
//    pthread_mutex_destroy(&source->audio_cb_mutex);
//    pthread_mutex_destroy(&source->audio_mutex);
//    pthread_mutex_destroy(&source->async_mutex);
//    obs_data_release(source->private_settings);
//    obs_context_data_free(&source->context);

//    if (source->owns_info_id) {
//        bfree((void *)source->info.id);
//    }

//    bfree(source);
}

bool lite_source::is_audio_source()
{
    return d_ptr->impl->output_flags & OBS_SOURCE_AUDIO;
}

void lite_source::allocate_audio_output_buffer()
{
    size_t size = sizeof(float) * AUDIO_OUTPUT_FRAMES * MAX_AUDIO_CHANNELS *
            MAX_AUDIO_MIXES;
    float *ptr = (float *)malloc(size);
    memset(ptr, 0, size);

    for (size_t mix = 0; mix < MAX_AUDIO_MIXES; mix++) {
        size_t mix_pos = mix * AUDIO_OUTPUT_FRAMES * MAX_AUDIO_CHANNELS;

        for (size_t i = 0; i < MAX_AUDIO_CHANNELS; i++) {
            d_ptr->audio_output_buf[mix][i] =
                    ptr + mix_pos + AUDIO_OUTPUT_FRAMES * i;
        }
    }
}
