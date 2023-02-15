#include "lite_ffmpeg_mux.h"
#include "util/circlebuf.h"
#include <atomic>
#include <list>
#include <thread>

struct lite_ffmpeg_mux_private
{
    int64_t stop_ts{};
    uint64_t total_bytes{};
    std::string path{};
    bool sent_headers{};
    std::atomic_bool active;
    std::atomic_bool stopping;
    std::atomic_bool capturing;

    /* replay buffer */
    circlebuf packets{};
    int64_t cur_size{};
    int64_t cur_time{};
    int64_t max_size{};
    int64_t max_time{};
    int64_t save_ts{};
    int keyframes{};

    std::list<std::shared_ptr<encoder_packet>> mux_packets{};
    std::thread mux_thread;
    std::atomic_bool muxing{};
};

lite_ffmpeg_mux::lite_ffmpeg_mux()
    : lite_obs_output()
{
    d_ptr = std::make_unique<lite_ffmpeg_mux_private>();
}

lite_ffmpeg_mux::~lite_ffmpeg_mux()
{

}
