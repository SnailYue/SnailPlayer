//
// Created by surface on 9/22/2020.
//

#ifndef SNAILPLAYER_NATIVE_WINDOW_RENDER_H
#define SNAILPLAYER_NATIVE_WINDOW_RENDER_H


#include <stdint.h>
#include <jni.h>

extern "C" {
#include <libavutil/frame.h>
}

class VideoDataProvider {
public:
    virtual void GetData(uint8_t **buffer, AVFrame **frame, int &width, int &height) = 0;

    virtual int GetVideoWidth() = 0;

    virtual int GetVideoHeight() = 0;

    virtual ~VideoDataProvider() = default;
};

void initVideoRender(JNIEnv *env, VideoDataProvider *provider, jobject surface);

void videoRender();

#endif //SNAILPLAYER_NATIVE_WINDOW_RENDER_H
