//
// Created by surface on 9/22/2020.
//

#include "native_window_render.h"
#include "log_util.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>

ANativeWindow *nativeWindow;
ANativeWindow_Buffer windowBuffer;
VideoDataProvider *videoDataProvider = nullptr;

/**
 * 初始化视频Window
 * @param env
 * @param provider
 * @param surface
 */
void initVideoRender(JNIEnv *env, VideoDataProvider *provider, jobject surface) {
    videoDataProvider = provider;
    nativeWindow = ANativeWindow_fromSurface(env, surface);
}

/**
 * 视频渲染
 */
void videoRender() {
    if (videoDataProvider == nullptr) {
        ELOG("Video render not init")
        return;
    }
    if (0 > ANativeWindow_setBuffersGeometry(nativeWindow, videoDataProvider->GetVideoWidth(),
                                             videoDataProvider->GetVideoHeight(),
                                             WINDOW_FORMAT_RGBA_8888)) {
        ELOG("Could not set buffer geometry")
        return;
    }
    while (true) {
        if (ANativeWindow_lock(nativeWindow, &windowBuffer, nullptr) < 0) {
            ELOG("Could not lock window")
        } else {
            uint8_t *buffer = nullptr;
            AVFrame *frame = nullptr;
            int width, height;
            videoDataProvider->GetData(&buffer, &frame, width, height);
            if (buffer == nullptr) {
                break;
            }
            uint8_t *dst = (uint8_t *) windowBuffer.bits;
            for (int i = 0; i < height; ++i) {
                memcpy(dst + i * windowBuffer.stride * 4, buffer + i * frame->linesize[0],
                       frame->linesize[0]);
            }
            ANativeWindow_unlockAndPost(nativeWindow);
        }
    }
}