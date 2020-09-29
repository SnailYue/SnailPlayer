//
// Created by surface on 9/29/2020.
//

#include "time_callback.h"
#include "log_util.h"


void PlayerTimeCallback::PlayTimeListener(double time, double total) {
    JNIEnv *env;
    bool need_detach = false;
    int getEnvStart = vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (getEnvStart == JNI_EDETACHED) {
        if (vm->AttachCurrentThread(&env, nullptr) != 0) {
            return;
        }
        need_detach = true;
    }
    jclass javaClass = env->GetObjectClass(cb);
    if (javaClass == 0) {
        ELOG("Unable to find class")
        vm->DetachCurrentThread();
        return;
    }
    jmethodID javaCallbackId = env->GetMethodID(javaClass, "playTime", "(DD)V");
    if (javaCallbackId == nullptr) {
        ELOG("Unable to find method")
        return;
    }
    env->CallVoidMethod(cb, javaCallbackId,time,total);
    if (need_detach) {
        vm->DetachCurrentThread();
    }
    env = nullptr;
}

void PlayerTimeCallback::PlayStateListener(int state) {
    JNIEnv *env;
    bool need_detach = false;
    int getEnvStart = vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (getEnvStart == JNI_EDETACHED) {
        if (vm->AttachCurrentThread(&env, nullptr) != 0) {
            return;
        }
        need_detach = true;
    }
    jclass javaClass = env->GetObjectClass(cb);
    if (javaClass == 0) {
        ELOG("Unable to find class")
        vm->DetachCurrentThread();
        return;
    }
    jmethodID javaCallbackId = env->GetMethodID(javaClass, "playState", "(I)V");
    if (javaCallbackId == nullptr) {
        ELOG("Unable to find method")
        return;
    }
    env->CallVoidMethod(cb, javaCallbackId, state);
    if (need_detach) {
        vm->DetachCurrentThread();
    }
    env = nullptr;
}

PlayerTimeCallback::PlayerTimeCallback(JNIEnv *env, jobject jcallback) {
    env->GetJavaVM(&vm);
    cb = env->NewGlobalRef(jcallback);
}