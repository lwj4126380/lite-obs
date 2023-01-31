#include "lite_obs.h"
#include "util/log.h"
#include <mutex>
#include <list>

#include "graphics/graphics.h"

lite_obs obs;

struct lite_obs_data
{
    std::recursive_mutex sources_mutex;
    std::recursive_mutex audio_sources_mutex;

    std::list<std::shared_ptr<lite_source>> sources;
    std::list<std::shared_ptr<lite_source>> audio_sources;
};

lite_obs::lite_obs()
{
    data = std::make_unique<lite_obs_data>();
}

void lite_obs::add_source(std::shared_ptr<lite_source> source, bool is_audio_source)
{
    if (is_audio_source) {
        std::lock_guard<std::recursive_mutex> lock(data->audio_sources_mutex);
        data->audio_sources.push_back(source);
    }

    std::lock_guard<std::recursive_mutex> lock(data->sources_mutex);
    data->sources.push_back(source);
}

void lite_obs::remove_source(std::shared_ptr<lite_source> source)
{
    {
        std::lock_guard<std::recursive_mutex> lock(data->audio_sources_mutex);
        for (auto iter = data->audio_sources.begin(); iter != data->audio_sources.end(); iter++) {
            if (source == *iter) {
                data->audio_sources.erase(iter);
                break;
            }
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock(data->sources_mutex);
        for (auto iter = data->sources.begin(); iter != data->sources.end(); iter++) {
            if (source == *iter) {
                data->sources.erase(iter);
                break;
            }
        }
    }
}

int lite_obs::obs_reset_video(obs_video_info *ovi)
{
    return obs_init_graphics(ovi);
}
#include <thread>
int lite_obs::obs_init_graphics(obs_video_info *ovi)
{
    std::thread th([](){

        qDebug() <<"FFFFFF " << sizeof(glm::vec3);
        auto gl = std::make_unique<graphics_subsystem>();
        qDebug() <<"+++++++++++ " << gl->gs_create();
    });

    th.join();
    /*
//             * Create an OpenGL framebuffer as render target.
//             */
//            GLuint frameBuffer;
//            glGenFramebuffers(1, &frameBuffer);
//            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
//            assertOpenGLError("glBindFramebuffer");


//            /*
//             * Create a texture as color attachment.
//             */
//            GLuint t;
//            glGenTextures(1, &t);

//            glBindTexture(GL_TEXTURE_2D, t);
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 500, 500, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//            assertOpenGLError("glTexImage2D");

//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


//            /*
//             * Attach the texture to the framebuffer.
//             */
//            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t, 0);
//            assertOpenGLError("glFramebufferTexture2D");


//            /*
//             * Render something.
//             */
//            glClearColor(1.0, 0.0, 0.0, 1.0);
//            glClear(GL_COLOR_BUFFER_BIT);
//            glFlush();

//            char *image_data = (char*)malloc(500 * 500 * 4);
//            memset(image_data, 0, 500 * 500 * 4);
//            //glReadBuffer(GL_COLOR_ATTACHMENT0);
//            glReadPixels(0, 0, 500, 500, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
//            assertOpenGLError("glReadPixels");

//            QDir dir;
//            dir.mkpath("/storage/emulated/0/Android/data/org.qtproject.example.lite_obs/cache");
//            QFile f("/storage/emulated/0/Android/data/org.qtproject.example.lite_obs/files/500.rgba");
//            f.open(QFile::ReadWrite);
//            f.write(image_data, 500*500*4);
//            f.close();

//            free(image_data);

//            /*
//             * Destroy context.
//             */
//            glDeleteFramebuffers(1, &frameBuffer);
//            glDeleteTextures(1, &t);


    return true;
}
