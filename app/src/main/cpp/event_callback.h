//
// Created by surface on 9/23/2020.
//

#ifndef SNAILPLAYER_EVENT_CALLBACK_H
#define SNAILPLAYER_EVENT_CALLBACK_H


#include <jni.h>

class EventCallback {
public:
    virtual void OnPrepared() = 0;

    virtual ~EventCallback() = default;
};


class PlayerEventCallback : public EventCallback {
public:
    PlayerEventCallback(JNIEnv *env, jobject jcallback);

    virtual void OnPrepared() override;

private:
    JavaVM *vm;
    jobject cb;
};

#endif //SNAILPLAYER_EVENT_CALLBACK_H
