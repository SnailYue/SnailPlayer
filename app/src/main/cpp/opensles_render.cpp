//
// Created by surface on 9/22/2020.
//

#include "opensles_render.h"
#include "log_util.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>

/**
 * object & interface
 */
SLObjectItf engineObject; //引擎对象
SLEngineItf engineEngine; //引擎接口

/**
 * output mix interface
 */
SLObjectItf outputMixObject; //混音器对象
SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL; //具体的混音器实例对象

/**
 * buffer queue player interface
 */
SLObjectItf bufferQueuePlayerObject;  //播放器接口对象
SLPlayItf bufferQueuePlayerPlay;  //具体播放器实例对象
SLAndroidSimpleBufferQueueItf bufferQueuePlayerBufferQueue;  //播放器的缓冲队列接口
SLEffectSendItf bufferQueuePlayerEffectSend;  //音效发送接口
SLMuteSoloItf bufferQueuePlayerMuteSolo;  //声道接口
SLVolumeItf bufferQueuePlayerVolume;  // 音量接口
SLmilliHertz bufferQueuePlayerSampleRate = 0; //采样率

pthread_mutex_t audioEngineLock = PTHREAD_MUTEX_INITIALIZER;

const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;


AudioDataProvider *audioDataProvider;

void bufferQueuePlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context) {
    ILOG("bufferQueuePlayerCallback")
    assert(bufferQueue == bufferQueuePlayerBufferQueue);
    if (audioDataProvider != NULL) {
        uint8_t *buffer;
        int size;
        audioDataProvider->GetData(&buffer, size);
        if (NULL != buffer && 0 != size) {
            SLresult result;
            /**
             * enqueue another buffer
             */
            result = (*bufferQueuePlayerBufferQueue)->Enqueue(bufferQueuePlayerBufferQueue, buffer,
                                                              size);
            if (SL_RESULT_SUCCESS != result) {
                pthread_mutex_unlock(&audioEngineLock);
            }
            (void) result;
        } else {
            pthread_mutex_unlock(&audioEngineLock);
        }
    }
}

/**
 * 创建音频播放的引擎
 */
void createAudioEngine() {
    ILOG("createAudioEngine")
    SLresult result;

    /**
     * create engine
     */
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    /**
     * realize engine
     */
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    /**
     * get engine interface
     */
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    /**
     * create output mix
     */
    const SLInterfaceID slInterfaceId[1] = {SL_IID_ENVIRONMENTALREVERB};  //环境混响音效
    const SLboolean slInterfaceRequired[1] = {SL_BOOLEAN_FALSE};  //混响音效是否必需
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, slInterfaceId,
                                              slInterfaceRequired);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    /**
     * realize output mix
     */
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    /**
     * get environmentalreverb interface
     */
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        /**
         * set output mix environmentalreverb properties
         */
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }
}


/**
 * 创建音频播放器中的缓冲队列
 * @param sampleRate
 * @param channel
 */
void createBufferQueueAudioPlayer(int sampleRate, int channel) {
    ILOG("createBufferQueueAudioPlayer")
    SLresult result;
    if (sampleRate >= 0) {
        bufferQueuePlayerSampleRate = sampleRate * 1000;
    }

    /**
     * configure input audio source
     */
    SLDataLocator_AndroidSimpleBufferQueue locatorAndroidSimpleBufferQueue = {   //缓冲队列定位器
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    //数据格式 pcm格式，单声道，采样率8khz，采样大小16bit，容器大小16bit，前中扩音，尾序
    SLDataFormat_PCM formatPcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
                                  SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                  SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    /**
     * set bufferqueue sample rate
     */
    if (bufferQueuePlayerSampleRate) {
        formatPcm.samplesPerSec = bufferQueuePlayerSampleRate;
    }
    /**
     * set bufferqueue channels
     */
    formatPcm.numChannels = (SLuint32) channel;
    if (channel == 2) {
        formatPcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    } else {
        formatPcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    }
    /**
     * Data source
     */
    SLDataSource audioSource = {&locatorAndroidSimpleBufferQueue, &formatPcm};
    SLDataLocator_OutputMix locatorOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    /**
     * Data sink
     */
    SLDataSink audioSink = {&locatorOutputMix, NULL};
    //缓冲队列，功能音量，功能音效
    const SLInterfaceID slInterfaceId[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND};
    const SLboolean slInterfaceRequired[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    /**
     * create audio player
     */
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bufferQueuePlayerObject,
                                                &audioSource, &audioSink,
                                                bufferQueuePlayerSampleRate ? 2 : 3, slInterfaceId,
                                                slInterfaceRequired);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    /**
     * realize player
     */
    result = (*bufferQueuePlayerObject)->Realize(bufferQueuePlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    /**
     * get play interface 获取播放接口
     */
    result = (*bufferQueuePlayerObject)->GetInterface(bufferQueuePlayerObject, SL_IID_PLAY,
                                                      &bufferQueuePlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    /**
     * get buffer queue interface 获取缓冲区接口
     */
    result = (*bufferQueuePlayerObject)->GetInterface(bufferQueuePlayerObject, SL_IID_BUFFERQUEUE,
                                                      &bufferQueuePlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    /**
     * register callback on the buffer queue  注册缓冲区的回调
     */
    result = (*bufferQueuePlayerBufferQueue)->RegisterCallback(bufferQueuePlayerBufferQueue,
                                                               bufferQueuePlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    /**
     * get the effect send interfae  获取音效接口
     */
    bufferQueuePlayerEffectSend = NULL;
    if (0 == bufferQueuePlayerSampleRate) {
        result = (*bufferQueuePlayerObject)->GetInterface(bufferQueuePlayerObject,
                                                          SL_IID_EFFECTSEND,
                                                          &bufferQueuePlayerEffectSend);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;
    }
#if 0
    result = (*bufferQueuePlayerObject)->GetInterface(bufferQueuePlayerObject, SL_IID_MUTESOLO, &bufferQueuePlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
#endif
    /**
     * get volume interface  获取音量接口
     */
    result = (*bufferQueuePlayerObject)->GetInterface(bufferQueuePlayerObject, SL_IID_VOLUME,
                                                      &bufferQueuePlayerVolume);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
}


void initAudioPlayer(int sampleRate, int channel, AudioDataProvider *p) {
    ILOG("initAudioPlayer")
    audioDataProvider = p;
    createAudioEngine();
    createBufferQueueAudioPlayer(sampleRate, channel);
}

void startAudioPlay() {
    ILOG("startAudioPlay")
    /**
     * set play state to playing  将播放状态改成正在播放
     */
    SLresult result = (*bufferQueuePlayerPlay)->SetPlayState(bufferQueuePlayerPlay,
                                                             SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    bufferQueuePlayerCallback(bufferQueuePlayerBufferQueue, NULL);
}

