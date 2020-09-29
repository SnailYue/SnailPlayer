//
// Created by surface on 9/29/2020.
//

#ifndef SNAILPLAYER_TIME_CALLBACK_H
#define SNAILPLAYER_TIME_CALLBACK_H


#include <jni.h>

class PlayerTimeCallback {
public:

    PlayerTimeCallback(JNIEnv *env, jobject jcallback);

    void PlayTimeListener(double time, double total);

    void PlayStateListener(int state);

private:
    JavaVM *vm;
    jobject cb;
};


#endif //SNAILPLAYER_TIME_CALLBACK_H
