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

#platform deps
if(WIN32)
    add_subdirectory("${CMAKE_SOURCE_DIR}/third-party/glad")

    set(FFMPEG_PLATFORM_NAME windows)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FFMPEG_LIB_PATH ${CMAKE_SOURCE_DIR}/third-party/ffmpeg/${FFMPEG_PLATFORM_NAME}/lib/x64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(FFMPEG_LIB_PATH ${CMAKE_SOURCE_DIR}/third-party/ffmpeg/${FFMPEG_PLATFORM_NAME}/lib/x86)
    endif()
    set(FFMPEG_LIBS ${FFMPEG_LIB_PATH}/ffmpeg.lib)
    set(THIRD_PARTY_LIBS ${FFMPEG_LIBS} glad)
elseif(ANDROID)
    set(FFMPEG_PLATFORM_NAME android)
    set(FFMPEG_LIB_PATH ${CMAKE_SOURCE_DIR}/third-party/ffmpeg/${FFMPEG_PLATFORM_NAME}/lib/${ANDROID_ABI})
    set(FFMPEG_LIBS
        ${FFMPEG_LIB_PATH}/libffmpeg.so)

    set(THIRD_PARTY_LIBS ${FFMPEG_LIBS})
endif()

include_directories(${CMAKE_SOURCE_DIR}/third-party/ffmpeg/${FFMPEG_PLATFORM_NAME}/include)


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
    media-io/media-io-defs.h

    media-io/audio_info.h
    media-io/audio_resampler.h
    media-io/audio_output.h

    media-io/video_info.h
    media-io/video_scaler.h
    media-io/video_frame.h
    media-io/video_output.h
    media-io/video-matrices.h

    lite_source.h
    lite_output.h
    lite_encoder.h

    lite_obs_core_video.h


    obs-defs.h
    lite-obs2_global.h
    lite_obs_info.h
    lite_obs.h
)

set(liteobs_SOURCES
    media-io/audio_resampler.cpp
    media-io/audio_output.cpp

    media-io/video_scaler.cpp
    media-io/video_frame.cpp
    media-io/video_output.cpp
    media-io/video-matrices.cpp

    lite_source.cpp
    lite_output.cpp
    lite_encoder.cpp

    lite_obs_core_video.cpp

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
    graphics/gs_texture_render.h
    graphics/gs_shader_info.h
    graphics/shaders.h
    graphics/gs_shader.h
    graphics/gs_program.h
    graphics/gs_stagesurf.h
)

set(liteobs_graphics_SOURCES
    graphics/gl-helpers.cpp
    graphics/gs_subsystem.cpp
    graphics/gs_device.cpp
    graphics/gs_vertexbuffer.cpp
    graphics/gs_texture.cpp
    graphics/gs_texture_render.cpp
    graphics/gs_shader.cpp
    graphics/shaders.cpp
    graphics/gs_program.cpp
    graphics/gs_stagesurf.cpp
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


target_link_libraries(lite-obs
    PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick
    ${THIRD_PARTY_LIBS}
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

    set_property(TARGET lite-obs PROPERTY QT_ANDROID_EXTRA_LIBS
        ${FFMPEG_LIBS})
endif()

target_compile_definitions(lite-obs PRIVATE LITEOBS_LIBRARY)

install(TARGETS lite-obs
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(lite-obs)
    qt_finalize_executable(lite-obs)
endif()

