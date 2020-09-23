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
    std::condition_variable ready;
    std::condition_variable full;
    const size_t maxSize = 16;
};

class FrameQueue {
public:
    void Put(AVFrame *frame);

    AVFrame *Get();

    size_t Size() const {
        return queue.size();
    };
private:
    std::queue<AVFrame *> queue;
    std::mutex mutex;
    std::condition_variable cond;
    const size_t maxSize = 16;
    const size_t readySize = 8;
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

    virtual void GetData(uint8_t **buffer, int &buffer_size) override;

    virtual void GetData(uint8_t **buffer, AVFrame **frame, int &width, int &height) override;

    virtual int GetVideoWidth() override;

    virtual int GetVideoHeight() override;

private:
    void read();

    void decodeAudio();

    void decodeVideo();

    void openStream(int index);

private:

    PlayerEventCallback *eventCallback;
    State state;
    std::unique_ptr<std::thread> read_thread;
    std::unique_ptr<std::thread> audio_decode_thread;
    std::unique_ptr<std::thread> video_decode_thread;
    std::unique_ptr<std::thread> audio_render_thread;
    std::unique_ptr<std::thread> video_render_thread;

    AVFormatContext *avFormatContext;
    std::string path;
    PacketQueue audio_packet;
    PacketQueue video_packet;

    FrameQueue audio_frame;
    FrameQueue video_frame;

    int video_stream_index;
    int audio_stream_index;

    AVStream *audio_stream;
    AVStream *video_stream;

    struct SwrContext *swr_context;
    struct SwsContext *img_convert_context;

    AVCodecContext *audio_codec_context;
    AVCodecContext *video_codec_context;

    int eof;
    uint8_t *audio_buffer;
    AVFrame *frame_rgba;
    uint8_t *rgba_buffer;
    double audio_clock;
};


#endif //SNAILPLAYER_SNAIL_PLAYER_H
