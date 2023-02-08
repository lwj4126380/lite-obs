#pragma once

#include "media-io-defs.h"
#include "video_info.h"
#include <stdint.h>
#include <memory>
#include <vector>

class video_frame
{
public:
    video_frame();
    ~video_frame();

    void video_frame_init(video_format format, uint32_t width, uint32_t height);
    void video_frame_free();

    static void video_frame_copy(video_frame *dst, const video_frame *src, video_format format, uint32_t cy);

    std::vector<uint8_t *> data;
    uint32_t linesize[MAX_AV_PLANES]{};

private:
    std::vector<uint8_t> data_internal;
};
