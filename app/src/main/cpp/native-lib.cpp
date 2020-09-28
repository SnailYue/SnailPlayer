#include <jni.h>
#include "snail_player.h"
#include "log_util.h"

//
// Created by surface on 9/21/2020.
//

SnailPlayer *player = new SnailPlayer();
PlayerEventCallback *eventCallback;

/**
 * 设置回调
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1setEventCallback(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jobject callback) {
    if (eventCallback) {
        delete eventCallback;
    }
    eventCallback = new PlayerEventCallback(env, callback);
    player->SetEventCallback(eventCallback);
}

/**
 * 设置数据源
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1setDataSource(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jstring url) {
    player->SetDataSource(env->GetStringUTFChars(url, NULL));
}

/**
 * 异步
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1prepareAsync(JNIEnv *env,
                                                                           jobject thiz) {
    player->PrepareAsync();
}

/**
 * 开始播放
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1start(JNIEnv *env, jobject thiz) {
    player->Start();
}

/**
 * 暂停
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1pause(JNIEnv *env, jobject thiz) {
    player->Pause();
}

/**
 * 继续播放
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1resume(JNIEnv *env, jobject thiz) {
    player->Resume();
}

/**
 * 设置surface
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1setSurface(JNIEnv *env, jobject thiz,
                                                                         jobject surface) {
    initVideoRender(env, player, surface);
}

/**
 * 获取视频的高
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1getVideoHeight(JNIEnv *env,
                                                                             jobject thiz) {
    return player->GetVideoHeight();
}

/**
 * 获取视频的宽
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1getVideoWidth(JNIEnv *env,
                                                                            jobject thiz) {
    return player->GetVideoWidth();
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1getDuration(JNIEnv *env,
                                                                          jobject thiz) {
    return player->Duration();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1seek_1to(JNIEnv *env, jobject thiz,
                                                                       jint time) {
    player->SeekTo(time);
}