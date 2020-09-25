//
// Created by surface on 9/22/2020.
//

#ifndef SNAILPLAYER_OPENSLES_RENDER_H
#define SNAILPLAYER_OPENSLES_RENDER_H


#include <cstdint>

class AudioDataProvider {
public:
    virtual void GetData(uint8_t **buffer, int &buffer_size) = 0;

    virtual ~AudioDataProvider() = default;
};

/**
 * 初始化音频播放器
 * @param sampleRate
 * @param channel
 * @param p
 */
void initAudioPlayer(int sampleRate, int channel, AudioDataProvider *p);

/**
 * 开始播放
 */
void startAudioPlay();

/**
 * 停止播放
 */
void stopAudioPlay();

/**
 * 继续播放
 */
void resumeAudioPlay();


#endif //SNAILPLAYER_OPENSLES_RENDER_H
