//
// Created by surface on 9/22/2020.
//

#include "opensles_render.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>

/**
 * object & interface
 */
SLObjectItf engineObject;
SLEngineItf engineEngine;

/**
 * output mix interface
 */
SLObjectItf outputMixObject;
SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

/**
 * buffer queue player interface
 */
SLObjectItf bufferQueuePlayerObject;
SLPlayItf bufferQueuePlayerPlay;
SLAndroidSimpleBufferQueueItf bufferQueuePlayerBufferQueue;
SLEffectSendItf bufferQueuePlayerEffectSend;
SLMuteSoloItf bufferQueuePlayerMuteSolo;
SLVolumeItf bufferQueuePlayerVolume;
SLmilliHertz bufferQueueSampleRate = 0;

pthread_mutex_t audioEngineLock = PTHREAD_MUTEX_INITIALIZER;

const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

const int outputBufferSize = 8196;

AudioDataProvider *audioDataProvider;

void bufferQueuePlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context) {
    assert(bufferQueue == bufferQueuePlayerBufferQueue);
    if (audioDataProvider != NULL) {
        uint8_t *buffer;
        int size;
        audioDataProvider->GetData(&buffer, size);
        if (buffer != NULL && 0 != size) {
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
    const SLInterfaceID slInterfaceId[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, slInterfaceId,
                                              req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    /**
     * realize output mix
     */
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    /**
     * get environmental reverb interface
     */
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
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
    SLresult result;
    if (sampleRate >= 0) {
        bufferQueueSampleRate = sampleRate * 1000;
    }

    /**
     * configure input audio source
     */
    SLDataLocator_AndroidSimpleBufferQueue locatorAndroidSimpleBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM formatPcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
                                  SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                  SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_BIGENDIAN};

    if (bufferQueueSampleRate) {
        formatPcm.samplesPerSec = bufferQueueSampleRate;
    }
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
    const SLInterfaceID slInterfaceId[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    /**
     * create audio player
     */
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bufferQueuePlayerObject,
                                                &audioSource, &audioSink,
                                                bufferQueueSampleRate ? 2 : 3, slInterfaceId, req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    /**
     * realize player
     */
    result = (*bufferQueuePlayerObject)->Realize(bufferQueuePlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    /**
     * get play interface
     */
    result = (*bufferQueuePlayerObject)->GetInterface(bufferQueuePlayerObject, SL_IID_PLAY,
                                                      &bufferQueuePlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    /**
     * get buffer queue interface
     */
    result = (*bufferQueuePlayerObject)->GetInterface(bufferQueuePlayerObject, SL_IID_BUFFERQUEUE,
                                                      &bufferQueuePlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    /**
     * register callback on the buffer queue
     */
    result = (*bufferQueuePlayerBufferQueue)->RegisterCallback(bufferQueuePlayerBufferQueue,
                                                               bufferQueuePlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    /**
     * get the effect send interfae
     */
    bufferQueuePlayerEffectSend = NULL;
    if (0 == bufferQueueSampleRate) {
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
     * get volume interface
     */
    result = (*bufferQueuePlayerObject)->GetInterface(bufferQueuePlayerObject, SL_IID_VOLUME,
                                                      &bufferQueuePlayerVolume);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
}


void initAudioPlayer(int sampleRate, int channel, AudioDataProvider *p) {
    audioDataProvider = p;
    createAudioEngine();
    createBufferQueueAudioPlayer(sampleRate, channel);
}

void startAudioPlay() {
    /**
     * set play state to playing
     */
    SLresult result = (*bufferQueuePlayerPlay)->SetPlayState(bufferQueuePlayerPlay,
                                                             SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    bufferQueuePlayerCallback(bufferQueuePlayerBufferQueue, NULL);
}

void stopAudioPlay() {
    /**
     * set play state to paused
     */
    SLresult result = (*bufferQueuePlayerPlay)->SetPlayState(bufferQueuePlayerPlay,
                                                             SL_PLAYSTATE_PAUSED);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
}
