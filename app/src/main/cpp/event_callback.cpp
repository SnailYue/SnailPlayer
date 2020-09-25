//
// Created by surface on 9/23/2020.
//

#include "event_callback.h"
#include "log_util.h"

/**
 * prepared函数
 */
void PlayerEventCallback::OnPrepared() {
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
    jmethodID javaCallbackId = env->GetMethodID(javaClass, "onPrepared", "()V");
    if (javaCallbackId == nullptr) {
        ELOG("Unable to find method")
        return;
    }
    env->CallVoidMethod(cb, javaCallbackId);
    if (need_detach) {
        vm->DetachCurrentThread();
    }
    env = nullptr;
}

/**
 * 构造函数
 * @param env
 * @param jcallback
 */
PlayerEventCallback::PlayerEventCallback(JNIEnv *env, jobject jcallback) {
    env->GetJavaVM(&vm);
    cb = env->NewGlobalRef(jcallback);
}