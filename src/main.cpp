#include "audio_output.h"
#include "audio_resampler.h"
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include <thread>
#include <QElapsedTimer>
#include <QFile>
#include "util/threading.h"
#include "video_frame.h"
#include "video_output.h"


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

//    audio_output *output = new audio_output;
//    audio_output_info info;
//    info.format = audio_format::AUDIO_FORMAT_16BIT;
//    info.name = "Audio";
//    info.samples_per_sec = 44100;
//    info.speakers = speaker_layout::SPEAKERS_STEREO;

//    output->audio_output_open(&info);

//    video_frame frame;
//    frame.video_frame_init(video_format::VIDEO_FORMAT_BGRA, 100, 100);
//    uint8_t dd[5] = {0x1, 2, 3, 4, 5};
//    memcpy(frame.data[0], dd, 5);
//    video_frame *f = &frame;
//    video_data data;
//    data.frame = *f;

////    std::unique_ptr<audio_resampler> resampler = std::make_unique<audio_resampler>();
////    resample_info dst;
////    dst.format = audio_format::AUDIO_FORMAT_FLOAT;
////    dst.samples_per_sec = 44100;
////    dst.speakers = speaker_layout::SPEAKERS_STEREO;

////    resample_info src;
////    src.format = audio_format::AUDIO_FORMAT_16BIT;
////    src.samples_per_sec = 48000;
////    src.speakers = speaker_layout::SPEAKERS_MONO;
////    qDebug() << resampler->create(&dst, &src);

////    QFile f("C:\\Users\\luweijia\\Desktop\\48000_2_s16le.pcm");
////    f.open(QFile::ReadOnly);
////    QFile out("D:\\44100_2_float.pcm");
////    out.open(QFile::ReadWrite);
////    int cc = 1024 * 2;
////    while(true) {
////        auto bytes = f.read(cc);
////        if (bytes.size() != cc)
////            break;

////        uint8_t *output[MAX_AV_PLANES];
////        uint32_t frames;
////        uint64_t offset;

////        uint8_t *input[MAX_AV_PLANES] = { nullptr };
////        input[0] = (uint8_t *)bytes.data();

////        memset(output, 0, sizeof(output));

////        resampler->do_resample(output, &frames, &offset, (const uint8_t *const *)input, 1024);
////        out.write((char *)output[0], frames * 2 * 4);
////    }
////    out.close();
////    f.close();

//    output->audio_output_close();
//    delete output;

//    return 0;
    return app.exec();
}
