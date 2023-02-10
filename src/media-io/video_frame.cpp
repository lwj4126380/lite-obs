#include "video_frame.h"

#define ALIGN_SIZE(size, align) size = (((size) + (align - 1)) & (~(align - 1)))

video_frame::video_frame()
{
    data.resize(8);
}

video_frame::~video_frame()
{

}

void video_frame::video_frame_init(video_format format, uint32_t width, uint32_t height)
{
    data.resize(MAX_AV_PLANES);

    size_t size;
    size_t offsets[MAX_AV_PLANES];
    int alignment = 32;

    memset(offsets, 0, sizeof(offsets));

    switch (format) {
    case video_format::VIDEO_FORMAT_NONE:
        return;

    case video_format::VIDEO_FORMAT_I420:
        size = width * height;
        ALIGN_SIZE(size, alignment);
        offsets[0] = size;
        size += (width / 2) * (height / 2);
        ALIGN_SIZE(size, alignment);
        offsets[1] = size;
        size += (width / 2) * (height / 2);
        ALIGN_SIZE(size, alignment);
        data_internal.resize(size);
        data[0] = data_internal.data();
        data[1] = data[0] + offsets[0];
        data[2] = data[0] + offsets[1];
        linesize[0] = width;
        linesize[1] = width / 2;
        linesize[2] = width / 2;
        break;

    case video_format::VIDEO_FORMAT_NV12:
        size = width * height;
        ALIGN_SIZE(size, alignment);
        offsets[0] = size;
        size += (width / 2) * (height / 2) * 2;
        ALIGN_SIZE(size, alignment);
        data_internal.resize(size);
        data[0] = data_internal.data();
        data[1] = data[0] + offsets[0];
        linesize[0] = width;
        linesize[1] = width;
        break;

    case video_format::VIDEO_FORMAT_Y800:
        size = width * height;
        ALIGN_SIZE(size, alignment);
        data_internal.resize(size);
        data[0] = data_internal.data();
        linesize[0] = width;
        break;

    case video_format::VIDEO_FORMAT_YVYU:
    case video_format::VIDEO_FORMAT_YUY2:
    case video_format::VIDEO_FORMAT_UYVY:
        size = width * height * 2;
        ALIGN_SIZE(size, alignment);
        data_internal.resize(size);
        data[0] = data_internal.data();
        linesize[0] = width * 2;
        break;

    case video_format::VIDEO_FORMAT_RGBA:
    case video_format::VIDEO_FORMAT_BGRA:
    case video_format::VIDEO_FORMAT_BGRX:
    case video_format::VIDEO_FORMAT_AYUV:
        size = width * height * 4;
        ALIGN_SIZE(size, alignment);
        data_internal.resize(size);
        data[0] = data_internal.data();
        linesize[0] = width * 4;
        break;

    case video_format::VIDEO_FORMAT_I444:
        size = width * height;
        ALIGN_SIZE(size, alignment);
        data_internal.resize(size * 3);
        data[0] = data_internal.data();
        data[1] = data[0] + size;
        data[2] = data[1] + size;
        linesize[0] = width;
        linesize[1] = width;
        linesize[2] = width;
        break;

    case video_format::VIDEO_FORMAT_BGR3:
        size = width * height * 3;
        ALIGN_SIZE(size, alignment);
        data_internal.resize(size);
        data[0] = data_internal.data();
        linesize[0] = width * 3;
        break;

    case video_format::VIDEO_FORMAT_I422:
        size = width * height;
        ALIGN_SIZE(size, alignment);
        offsets[0] = size;
        size += (width / 2) * height;
        ALIGN_SIZE(size, alignment);
        offsets[1] = size;
        size += (width / 2) * height;
        ALIGN_SIZE(size, alignment);
        data_internal.resize(size);
        data[0] = data_internal.data();
        data[1] = data[0] + offsets[0];
        data[2] = data[0] + offsets[1];
        linesize[0] = width;
        linesize[1] = width / 2;
        linesize[2] = width / 2;
        break;

    case video_format::VIDEO_FORMAT_I40A:
        size = width * height;
        ALIGN_SIZE(size, alignment);
        offsets[0] = size;
        size += (width / 2) * (height / 2);
        ALIGN_SIZE(size, alignment);
        offsets[1] = size;
        size += (width / 2) * (height / 2);
        ALIGN_SIZE(size, alignment);
        offsets[2] = size;
        size += width * height;
        ALIGN_SIZE(size, alignment);
        data_internal.resize(size);
        data[0] = data_internal.data();
        data[1] = data[0] + offsets[0];
        data[2] = data[0] + offsets[1];
        data[3] = data[0] + offsets[2];
        linesize[0] = width;
        linesize[1] = width / 2;
        linesize[2] = width / 2;
        linesize[3] = width;
        break;

    case video_format::VIDEO_FORMAT_I42A:
        size = width * height;
        ALIGN_SIZE(size, alignment);
        offsets[0] = size;
        size += (width / 2) * height;
        ALIGN_SIZE(size, alignment);
        offsets[1] = size;
        size += (width / 2) * height;
        ALIGN_SIZE(size, alignment);
        offsets[2] = size;
        size += width * height;
        ALIGN_SIZE(size, alignment);
        data_internal.resize(size);
        data[0] = data_internal.data();
        data[1] = data[0] + offsets[0];
        data[2] = data[0] + offsets[1];
        data[3] = data[0] + offsets[2];
        linesize[0] = width;
        linesize[1] = width / 2;
        linesize[2] = width / 2;
        linesize[3] = width;
        break;

    case video_format::VIDEO_FORMAT_YUVA:
        size = width * height;
        ALIGN_SIZE(size, alignment);
        offsets[0] = size;
        size += width * height;
        ALIGN_SIZE(size, alignment);
        offsets[1] = size;
        size += width * height;
        ALIGN_SIZE(size, alignment);
        offsets[2] = size;
        size += width * height;
        ALIGN_SIZE(size, alignment);
        data_internal.resize(size);
        data[0] = data_internal.data();
        data[1] = data[0] + offsets[0];
        data[2] = data[0] + offsets[1];
        data[3] = data[0] + offsets[2];
        linesize[0] = width;
        linesize[1] = width;
        linesize[2] = width;
        linesize[3] = width;
        break;
    }
}

void video_frame::video_frame_free()
{
    data_internal.clear();
    data.clear();
    memset(linesize, 0, sizeof(linesize));
}

void video_frame::video_frame_copy(video_frame *dst, const video_frame *src, video_format format, uint32_t cy)
{
    switch (format) {
        case video_format::VIDEO_FORMAT_NONE:
            return;

        case video_format::VIDEO_FORMAT_I420:
            memcpy(dst->data[0], src->data[0], src->linesize[0] * cy);
            memcpy(dst->data[1], src->data[1], src->linesize[1] * cy / 2);
            memcpy(dst->data[2], src->data[2], src->linesize[2] * cy / 2);
            break;

        case video_format::VIDEO_FORMAT_NV12:
            memcpy(dst->data[0], src->data[0], src->linesize[0] * cy);
            memcpy(dst->data[1], src->data[1], src->linesize[1] * cy / 2);
            break;

        case video_format::VIDEO_FORMAT_Y800:
        case video_format::VIDEO_FORMAT_YVYU:
        case video_format::VIDEO_FORMAT_YUY2:
        case video_format::VIDEO_FORMAT_UYVY:
        case video_format::VIDEO_FORMAT_RGBA:
        case video_format::VIDEO_FORMAT_BGRA:
        case video_format::VIDEO_FORMAT_BGRX:
        case video_format::VIDEO_FORMAT_BGR3:
        case video_format::VIDEO_FORMAT_AYUV:
            memcpy(dst->data[0], src->data[0], src->linesize[0] * cy);
            break;

        case video_format::VIDEO_FORMAT_I444:
        case video_format::VIDEO_FORMAT_I422:
            memcpy(dst->data[0], src->data[0], src->linesize[0] * cy);
            memcpy(dst->data[1], src->data[1], src->linesize[1] * cy);
            memcpy(dst->data[2], src->data[2], src->linesize[2] * cy);
            break;

        case video_format::VIDEO_FORMAT_I40A:
            memcpy(dst->data[0], src->data[0], src->linesize[0] * cy);
            memcpy(dst->data[1], src->data[1], src->linesize[1] * cy / 2);
            memcpy(dst->data[2], src->data[2], src->linesize[2] * cy / 2);
            memcpy(dst->data[3], src->data[3], src->linesize[3] * cy);
            break;

        case video_format::VIDEO_FORMAT_I42A:
        case video_format::VIDEO_FORMAT_YUVA:
            memcpy(dst->data[0], src->data[0], src->linesize[0] * cy);
            memcpy(dst->data[1], src->data[1], src->linesize[1] * cy);
            memcpy(dst->data[2], src->data[2], src->linesize[2] * cy);
            memcpy(dst->data[3], src->data[3], src->linesize[3] * cy);
            break;
        }
}
