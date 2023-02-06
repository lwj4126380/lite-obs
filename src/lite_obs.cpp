#include "lite_obs.h"
#include "util/log.h"
#include <mutex>
#include <list>

#include "graphics/gs_subsystem.h"
#include "graphics/gs_texture.h"
#include "graphics/gs_program.h"

lite_obs obs;

struct lite_obs_data
{
    std::recursive_mutex sources_mutex;
    std::recursive_mutex audio_sources_mutex;

    std::list<std::shared_ptr<lite_source>> sources{};
    std::list<std::shared_ptr<lite_source>> audio_sources{};
};

struct lite_obs_core_video
{
    std::unique_ptr<graphics_subsystem> graphics{};
};

struct lite_obs_private
{
    struct lite_obs_data data{};
    struct lite_obs_core_video video{};
};

lite_obs::lite_obs()
{
    d_ptr = std::make_unique<lite_obs_private>();
}

void lite_obs::add_source(std::shared_ptr<lite_source> source, bool is_audio_source)
{
    if (is_audio_source) {
        std::lock_guard<std::recursive_mutex> lock(d_ptr->data.audio_sources_mutex);
        d_ptr->data.audio_sources.push_back(source);
    }

    std::lock_guard<std::recursive_mutex> lock(d_ptr->data.sources_mutex);
    d_ptr->data.sources.push_back(source);
}

void lite_obs::remove_source(std::shared_ptr<lite_source> source)
{
    {
        std::lock_guard<std::recursive_mutex> lock(d_ptr->data.audio_sources_mutex);
        for (auto iter = d_ptr->data.audio_sources.begin(); iter != d_ptr->data.audio_sources.end(); iter++) {
            if (source == *iter) {
                d_ptr->data.audio_sources.erase(iter);
                break;
            }
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock(d_ptr->data.sources_mutex);
        for (auto iter = d_ptr->data.sources.begin(); iter != d_ptr->data.sources.end(); iter++) {
            if (source == *iter) {
                d_ptr->data.sources.erase(iter);
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
#include <QFile>
#include <QImage>
int lite_obs::obs_init_graphics(obs_video_info *ovi)
{
    std::thread th([](){
        auto gl = std::make_unique<graphics_subsystem>();
        auto errcode = gl->gs_create();
        if (errcode != GS_SUCCESS) {
            blog(LOG_DEBUG, "obs_init_graphics fail.");
            return;
        }

        //d_ptr->video.graphics = std::move(gl);
        gs_enter_contex(gl);

        gl->gs_effect_init();
//        {
//            uint8_t transparent_tex_data[2 * 2 * 4] = {0};
//            const uint8_t *transparent_tex = transparent_tex_data;
//            auto tex = gs_texture_create(2, 2, gs_color_format::GS_RGBA, 1, &transparent_tex, GS_DYNAMIC);
//            uint8_t *ptr;
//            uint32_t linesize_out;
//            blog(LOG_DEBUG, "++++++++++ %d", tex->gs_texture_map(&ptr, &linesize_out));
//            tex->gs_texture_unmap();
//        }

        QImage image(":/test.jpg");
        image = image.convertToFormat(QImage::Format_RGBA8888);

        auto bits = image.bits();
        auto img_tex = gs_texture_create(image.width(), image.height(), gs_color_format::GS_RGBA, 1, (const uint8_t **)&bits, 0);

        auto tex = gs_texture_create(640, 480, gs_color_format::GS_RGBA, 1, nullptr, GS_RENDER_TARGET);
        gs_set_render_target(tex, nullptr);


        glm::vec4 clear_color{0.0f, 0.0f, 1.0f, 0.0f};

        gs_clear(GS_CLEAR_COLOR, &clear_color, 1.0f, 0);

        gs_set_render_size(640, 480);

        auto program = gl->gs_get_effect_by_name("default_effect");
        program->gs_effect_set_texture("image", img_tex);

        gl->gs_draw_sprite(img_tex, 0, 1005, 792);

        glFlush();


        char *image_data = (char*)malloc(640 * 480 * 4);
        memset(image_data, 0, 640 * 480 * 4);
        //glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, image_data);



        QFile f("/storage/emulated/0/Android/data/org.qtproject.example.lite_obs/files/500.rgba");
        f.open(QFile::ReadWrite);
        f.write(image_data, 640*480*4);
        f.close();

        free(image_data);


        tex.reset();
        img_tex.reset();

        gs_leave_context();
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
