#include <jni.h>

//
// Created by surface on 9/21/2020.
//

/**
 * 设置回调
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1setEventCallback(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jobject callback) {

}

/**
 * 设置数据源
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1setDataSource(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jstring url) {

}

/**
 * 异步
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1prepareAsync(JNIEnv *env,
                                                                           jobject thiz) {

}

/**
 * 开始播放
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1start(JNIEnv *env, jobject thiz) {

}

/**
 * 暂停
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1pause(JNIEnv *env, jobject thiz) {

}

/**
 * 设置surface
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1setSurface(JNIEnv *env, jobject thiz,
                                                                         jobject surface) {

}

/**
 * 获取视频的高
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1getVideoHeight(JNIEnv *env,
                                                                             jobject thiz) {

}

/**
 * 获取视频的宽
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_snail_snailplayer_native_SnailPlayerNative__1native_1getVideoWidth(JNIEnv *env,
                                                                            jobject thiz) {

}

