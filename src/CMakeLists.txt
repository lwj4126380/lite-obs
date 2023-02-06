cmake_minimum_required(VERSION 3.14)

project(lite-obs LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
#set(DepsPath "D:/xxq-obs-deps")
#find_package(FFmpeg REQUIRED
#        COMPONENTS avformat avutil swscale swresample
#        OPTIONAL_COMPONENTS avcodec)
#include_directories(${FFMPEG_INCLUDE_DIRS})
include_directories("C:/Users/luweijia/Downloads/ffmpeg-4.4-android-lite.tar/ffmpeg-4.4-android-lite/include")
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third-party)

set(liteobs_util_HEADERS
    util/util_uint64.h
    util/util_uint128.h
    util/bmem.h
    util/threading.h
    util/log.h
    util/circlebuf.h
)

set(liteobs_util_SOURCES
    util/bmem.cpp
    util/threading.cpp
)

set(liteobs_HEADERS
    media-io-defs.h

#    audio_info.h
#    audio_resampler.h
#    audio_output.h

#    video_info.h
#    video_scaler.h
#    video_frame.h
#    video_output.h

    lite_source.h
    lite_output.h
    lite_encoder.h

    lite-obs2_global.h
    lite_obs.h
)

set(liteobs_SOURCES
#    audio_resampler.cpp
#    audio_output.cpp

#    video_scaler.cpp
#    video_frame.cpp
#    video_output.cpp

    lite_source.cpp
    lite_output.cpp
    lite_encoder.cpp

    lite_obs.cpp

    main.cpp
    qml.qrc
)

set(liteobs_graphics_HEADERS
    graphics/gs_subsystem_info.h
    graphics/gl-helpers.h
    graphics/gs_subsystem.h
    graphics/gs_device.h
    graphics/gs_vertexbuffer.h
    graphics/gs_texture.h
    graphics/gs_shader_info.h
    graphics/shaders.h
    graphics/gs_shader.h
    graphics/gs_program.h
)

set(liteobs_graphics_SOURCES
    graphics/gl-helpers.cpp
    graphics/gs_subsystem.cpp
    graphics/gs_device.cpp
    graphics/gs_vertexbuffer.cpp
    graphics/gs_texture.cpp
    graphics/gs_shader.cpp
    graphics/shaders.cpp
    graphics/gs_program.cpp
)

qt_add_executable(lite-obs
  MANUAL_FINALIZATION
  ${liteobs_util_HEADERS}
  ${liteobs_util_SOURCES}
  ${liteobs_graphics_HEADERS}
  ${liteobs_graphics_SOURCES}
  ${liteobs_HEADERS}
  ${liteobs_SOURCES}
)

if(MSVC)
    add_subdirectory("${CMAKE_SOURCE_DIR}/third-party/glad")
endif()

target_link_libraries(lite-obs
    PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick
#    "C:/Users/luweijia/Downloads/ffmpeg-4.4-android-lite.tar/ffmpeg-4.4-android-lite/bin/libswscale.so"
#    "C:/Users/luweijia/Downloads/ffmpeg-4.4-android-lite.tar/ffmpeg-4.4-android-lite/bin/libswresample.so"
#    "C:/Users/luweijia/Downloads/ffmpeg-4.4-android-lite.tar/ffmpeg-4.4-android-lite/bin/libavutil.so"
#    ${FFMPEG_LIBRARIES}
)


if(ANDROID)
    find_path(GLES3_INCLUDE_DIR GLES3/gl3.h
        HINTS ${ANDROID_NDK})

    if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
        set(ARCH "arm-linux-androideabi")
    endif()

    find_library(GLES3_LIBRARY libGLESv3.so
        HINTS ${GLES3_INCLUDE_DIR}/../lib/${ARCH}/${ANDROID_ABI})

    target_link_libraries(lite-obs PRIVATE ${GLES3_LIBRARY})
endif()

if(MSVC)
    target_link_libraries(lite-obs PRIVATE glad)
endif()

target_compile_definitions(lite-obs PRIVATE LITEOBS_LIBRARY)

install(TARGETS lite-obs
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(lite-obs)
    qt_finalize_executable(lite-obs)
endif()

