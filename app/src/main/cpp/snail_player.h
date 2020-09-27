//
// Created by surface on 9/23/2020.
//

#ifndef SNAILPLAYER_SNAIL_PLAYER_H
#define SNAILPLAYER_SNAIL_PLAYER_H

extern "C" {
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <queue>
#include <mutex>
#include "opensles_render.h"
#include "native_window_render.h"
#include "event_callback.h"
#include <thread>

#define AUDIO_BUFFER_SIZE 8196
#define AUDIO_READY_SIZE 8

/**
 * 播放状态
 */
enum class State {
    Idle,
    Initialized,
    Preparing,
    Prepared,
    Started,
    Paused,
    Stoped,
    Completed,
    End,
    Error,
};

/**
 * 包队列
 */
class PacketQueue {
public:

    void Put(AVPacket *packet);

    void Get(AVPacket *packet);

    void Clear();

    size_t Size();

private:
    std::queue<AVPacket> queue;
    int64_t duration;
    std::mutex mutex;
    std::condition_variable getCond;
    std::condition_variable putCond;
    const size_t maxSize = 16;
};

/**
 * 帧队列
 */
class FrameQueue {
public:
    void Put(AVFrame *frame);

    AVFrame *Get();

    size_t Size() const {
        return queue.size();
    };

    void SetPlayState(bool s);

private:
    std::queue<AVFrame *> queue;
    std::mutex mutex;
    std::condition_variable putCond;
    std::condition_variable getCond;
    const size_t maxSize = 16;
    bool isPause;
};


class SnailPlayer : public AudioDataProvider, public VideoDataProvider {
public:
    SnailPlayer();

    SnailPlayer(const SnailPlayer &) = delete;

    ~SnailPlayer();

    int SetDataSource(const std::string &p);

    int PrepareAsync();

    void SetEventCallback(PlayerEventCallback *cb);

    int Start();

    int Pause();

    int Resume();

    virtual void GetData(uint8_t **buffer, int &buffer_size) override;

    virtual void GetData(uint8_t **buffer, AVFrame **frame, int &width, int &height) override;

    virtual int GetVideoWidth() override;

    virtual int GetVideoHeight() override;

    long Duration();


private:
    void read();

    void decodeAudio();

    void decodeVideo();

    void openStream(int index);

private:

    PlayerEventCallback *eventCallback;
    State state;
    //读取线程
    std::unique_ptr<std::thread> read_thread;
    //音视频解码线程
    std::unique_ptr<std::thread> audio_decode_thread;
    std::unique_ptr<std::thread> video_decode_thread;
    //音视频渲染线程
    std::unique_ptr<std::thread> audio_render_thread;
    std::unique_ptr<std::thread> video_render_thread;
    //输出格式上下文
    AVFormatContext *avFormatContext;
    std::string path;
    //包队列
    PacketQueue audio_packet;
    PacketQueue video_packet;
    //帧队列
    FrameQueue audio_frame;
    FrameQueue video_frame;
    //音视频流通道索引
    int video_stream_index;
    int audio_stream_index;
    //音视频流
    AVStream *audio_stream;
    AVStream *video_stream;

    struct SwrContext *swr_context;
    struct SwsContext *img_convert_context;
    //音视频编解码器上下文
    AVCodecContext *audio_codec_context;
    AVCodecContext *video_codec_context;

    int eof;
    uint8_t *audio_buffer;
    AVFrame *frame_rgba;
    uint8_t *rgba_buffer;
    double audio_clock;
};


#endif //SNAILPLAYER_SNAIL_PLAYER_H
