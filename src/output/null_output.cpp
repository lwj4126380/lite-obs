#include "null_output.h"
#include <thread>
#include "util/log.h"

struct null_output_private
{
    std::thread stop_thread;
    bool stop_thread_active{};
    bool initilized{};
};

null_output::null_output()
{
    d_ptr = std::make_unique<null_output_private>();
}

bool null_output::i_output_valid()
{
    return d_ptr->initilized;
}

bool null_output::i_has_video()
{
    return true;
}

bool null_output::i_has_audio()
{
    return false;
}

bool null_output::i_encoded()
{
    return true;
}

bool null_output::i_create()
{
    d_ptr->initilized = true;
    return true;
}

void null_output::i_destroy()
{
    if (d_ptr->stop_thread_active && d_ptr->stop_thread.joinable())
        d_ptr->stop_thread.join();

    d_ptr->initilized = false;
}

bool null_output::i_start()
{
    if (!lite_obs_output_can_begin_data_capture())
        return false;
    if (!lite_obs_output_initialize_encoders())
        return false;

    if (d_ptr->stop_thread_active && d_ptr->stop_thread.joinable())
        d_ptr->stop_thread.join();

    lite_obs_output_begin_data_capture();
    return true;
}

void null_output::stop_thread(void *data)
{
    auto context = (null_output *)data;
    context->lite_obs_output_end_data_capture();
    context->d_ptr->stop_thread_active = false;
}

void null_output::i_stop(uint64_t ts)
{
    d_ptr->stop_thread = std::thread(stop_thread, this);
}

void null_output::i_raw_video(video_data *frame)
{

}

void null_output::i_raw_audio(audio_data *frames)
{

}

void null_output::i_encoded_packet(std::shared_ptr<encoder_packet> packet)
{
    blog(LOG_DEBUG, "======== %d", (int)packet->data.size());
}

uint64_t null_output::i_get_total_bytes()
{
    return 0;
}

int null_output::i_get_dropped_frames()
{
    return 0;
}