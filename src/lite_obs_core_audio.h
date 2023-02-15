#pragma once

#include <memory>

struct lite_obs_core_audio_private;
class audio_output;
class lite_obs_core_audio
{
public:
    lite_obs_core_audio();
    ~lite_obs_core_audio();

    std::shared_ptr<audio_output> core_audio();

    bool lite_obs_start_audio(const struct obs_audio_info *oai);
    void lite_obs_stop_audio();

private:
    bool audio_callback_internal(uint64_t start_ts_in, uint64_t end_ts_in,
                                 uint64_t *out_ts, uint32_t mixers,
                                 struct audio_output_data *mixes);
    static bool audio_callback(void *param, uint64_t start_ts_in, uint64_t end_ts_in,
                               uint64_t *out_ts, uint32_t mixers,
                               struct audio_output_data *mixes);

private:
    void free_audio();

private:
    std::unique_ptr<lite_obs_core_audio_private> d_ptr{};
};
