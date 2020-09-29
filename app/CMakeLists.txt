cmake_minimum_required(VERSION 3.4.1)

set(lib_src_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})

include_directories(
        ${CMAKE_SOURCE_DIR}/src/main/cpp/include
)

add_library(avcodec-57
        SHARED
        IMPORTED)
set_target_properties(avcodec-57
        PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavcodec-57.so)

add_library(avdevice-57
        SHARED
        IMPORTED)
set_target_properties(avdevice-57
        PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavdevice-57.so)

add_library(avfilter-6
        SHARED
        IMPORTED)
set_target_properties(avfilter-6
        PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavfilter-6.so)

add_library(avformat-57
        SHARED
        IMPORTED)
set_target_properties(avformat-57
        PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavformat-57.so)

add_library(avutil-55
        SHARED
        IMPORTED)
set_target_properties(avutil-55
        PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavutil-55.so)

add_library(postproc-54
        SHARED
        IMPORTED)
set_target_properties(postproc-54
        PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libpostproc-54.so)

add_library(swresample-2
        SHARED
        IMPORTED)
set_target_properties(swresample-2
        PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libswresample-2.so)

add_library(swscale-4
        SHARED
        IMPORTED)
set_target_properties(swscale-4
        PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libswscale-4.so)

add_library(native-lib SHARED
        ${CMAKE_SOURCE_DIR}/src/main/cpp/native-lib.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/native_window_render.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/log_util.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/opensles_render.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/event_callback.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/snail_player.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/time_callback.cpp
        )

target_link_libraries(native-lib
        avcodec-57
        avdevice-57
        avfilter-6
        avformat-57
        avutil-55
        postproc-54
        swresample-2
        swscale-4
        log
        android
        OpenSLES)
