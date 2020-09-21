cmake_minimum_required(VERSION 3.4.1)

find_library(log-lib log)

include_directories(
        ${CMAKE_SOURCE_DIR}/src/main/cpp/include
)

add_library(avcodec-57_lib SHARED IMPORTED)
set_target_properties(avcodec-57_lib PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libavcodec-57.so)


add_library(avformat-57_lib SHARED IMPORTED)
set_target_properties(avformat-57_lib PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libavformat-57.so)

add_library(avutil-55_lib SHARED IMPORTED)
set_target_properties(avutil-55_lib PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libavutil-55.so)

add_library(swresample-2_lib SHARED IMPORTED)
set_target_properties(swresample-2_lib PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libswresample-2.so)

add_library(swscale-4_lib SHARED IMPORTED)
set_target_properties(swscale-4_lib PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libswscale-4.so)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

# Include libraries needed for VideoPlayer lib
target_link_libraries(
        android
        OpenSLES
        avcodec-57_lib
        avformat-57_lib
        avutil-55_lib
        swresample-2_lib
        swscale-4_lib
        ${log-lib})
