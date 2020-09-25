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
    //获取surface
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
    //设置nativewindow绘制窗口的属性   nativeWindow指针，图像宽，图像高，像素的内存格式
    if (0 > ANativeWindow_setBuffersGeometry(nativeWindow, videoDataProvider->GetVideoWidth(),
                                             videoDataProvider->GetVideoHeight(),
                                             WINDOW_FORMAT_RGBA_8888)) {
        ELOG("Could not set buffer geometry")
        return;
    }
    while (true) {
        ILOG("video render")
        //获取buffer绘制缓冲区
        if (ANativeWindow_lock(nativeWindow, &windowBuffer, nullptr) < 0) {
            ELOG("Could not lock window")
        } else {
            //填充图像数据到buffer绘制缓冲区中
            uint8_t *buffer = nullptr;
            AVFrame *frame = nullptr;
            int width, height;
            //获取视频数据
            videoDataProvider->GetData(&buffer, &frame, width, height);
            if (buffer == nullptr) {
                ELOG("buffer is null,native window is over")
                break;
            }
            //数据位
            uint8_t *dst = (uint8_t *) windowBuffer.bits;
            /**
             * 一行一次拷贝   windowBuffer.stride * 4 每行的数据个数 * 4 表示RGBA数据个数
             * dst + i * windowBuffer.stride * 4 拷贝的目标地址
             * buffer + i * frame->linesize[0] 拷贝源地址
             */
            for (int i = 0; i < height; ++i) {
                memcpy(dst + i * windowBuffer.stride * 4, buffer + i * frame->linesize[0],
                       frame->linesize[0]);
            }
            //启动绘制
            ANativeWindow_unlockAndPost(nativeWindow);
        }
    }
}