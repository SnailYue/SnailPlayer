//
// Created by surface on 9/23/2020.
//

#include <__bit_reference>
#include <unistd.h>
#include "snail_player.h"
#include "log_util.h"
#include "common.h"

inline char *GetAVErrorMsg(int code) {
    static char err_msg[1024];
    av_strerror(code, err_msg, sizeof(err_msg));
    return err_msg;
}

/**
 * 往包队列中添加数据
 * @param packet
 */
void PacketQueue::Put(AVPacket *packet) {
    ILOG("PacketQueue::Put")
    std::unique_lock<std::mutex> lock(mutex);
    putCond.wait(lock, [this] {
        return queue.size() <= maxSize;
    });
    queue.push(*packet);
    getCond.notify_all();
}

/**
 * 获取包队列中的数据
 * @param packet
 */
void PacketQueue::Get(AVPacket *packet) {
    ILOG("PacketQueue::Get")
    if (packet == nullptr) {
        ELOG("packet is null")
        return;
    }
    std::unique_lock<std::mutex> lock(mutex);
    getCond.wait(lock, [this] {
        return !queue.empty();
    });
    *packet = queue.front();
    queue.pop();
    putCond.notify_all();
}

/**
 * 清空包队列
 */
void PacketQueue::Clear() {
    ILOG("PacketQueue::Clear")
    std::queue<AVPacket> empty;
    swap(empty, queue);
}

/**
 * 获取帧队列中的数据
 * @return
 */
AVFrame *FrameQueue::Get() {
    ILOG("FrameQueue::Get")
    std::unique_lock<std::mutex> lock(mutex);
    getCond.wait(lock, [this] {
        return !queue.empty();
    });
    AVFrame *frame = queue.front();
    queue.pop();
    putCond.notify_all();
    return frame;
}

/**
 * 往帧队列中添加数据
 * @param frame
 */
void FrameQueue::Put(AVFrame *frame) {
    ILOG("FrameQueue::Put")
    std::unique_lock<std::mutex> lock(mutex);
    putCond.wait(lock, [this] {
        return queue.size() <= maxSize;
    });
    AVFrame *temp = av_frame_alloc();
    av_frame_move_ref(temp, frame);
    queue.push(temp);
    getCond.notify_all();
}


SnailPlayer::SnailPlayer() : state(State::Idle), avFormatContext(nullptr), eof(0),
                             video_stream_index(-1), audio_stream_index(-1), eventCallback(nullptr),
                             audio_clock(0) {
    av_log_set_callback(ff_log_callback);
    av_log_set_level(AV_LOG_DEBUG);
    av_register_all();
    avformat_network_init();
}

SnailPlayer::~SnailPlayer() {
    if (avFormatContext) {
        avformat_free_context(avFormatContext);
        avFormatContext = nullptr;
    }
}

/**
 * 设置数据源
 * @param p
 * @return
 */
int SnailPlayer::SetDataSource(const std::string &p) {
    ILOG("SnailPlayer::SetDataSource path = %s", p.c_str())
    if (state != State::Idle) {
        ELOG("illegal state|current:%d", state)
        return ERROR_ILLEGAL_STATE;
    }
    path = p;
    ILOG("source path:%s", path.c_str())
    state = State::Initialized;
    return SUCCESS;
}

/**
 * 异步
 * @return
 */
int SnailPlayer::PrepareAsync() {
    ILOG("SnailPlayer::PrepareAsync")
    if (state != State::Initialized && state != State::Stoped) {
        ELOG("illegal state|current:%d", state);
        return ERROR_ILLEGAL_STATE;
    }
    state = State::Preparing;
    read_thread.reset(new std::thread(&SnailPlayer::read, this));
    return SUCCESS;
}

/**
 * 回调事件
 * @param cb
 */
void SnailPlayer::SetEventCallback(PlayerEventCallback *cb) {
    ILOG("SnailPlayer::SetEventCallback")
    eventCallback = cb;
}

/**
 * 获取音频数据
 * @param buffer
 * @param buffer_size
 */
void SnailPlayer::GetData(uint8_t **buffer, int &buffer_size) {
    ILOG("SnailPlayer::GetData Audio")
    if (audio_buffer == nullptr) {
        ELOG("audio buffer is null")
        return;
    }
    auto frame = audio_frame.Get();
    int next_size;
    if (audio_codec_context->sample_fmt == AV_SAMPLE_FMT_S16P) {
        next_size = av_samples_get_buffer_size(frame->linesize, audio_codec_context->channels,
                                               audio_codec_context->frame_size,
                                               audio_codec_context->sample_fmt, 1);
    } else {
        av_samples_get_buffer_size(&next_size, audio_codec_context->channels,
                                   audio_codec_context->frame_size, audio_codec_context->sample_fmt,
                                   1);
    }
    swr_convert(swr_context, &audio_buffer, frame->nb_samples,
                (uint8_t const **) (frame->extended_data), frame->nb_samples);
    //获取音频的时间基
    audio_clock = frame->pkt_dts * av_q2d(audio_stream->time_base);
    av_frame_unref(frame);
    av_frame_free(&frame);
    *buffer = audio_buffer;
    buffer_size = next_size;
}

/**
 * 打开数据源并读取相关的信息
 */
void SnailPlayer::read() {
    ILOG("SnailPlayer::read")
    int err;
    char err_buffer[1024];
    err = avformat_open_input(&avFormatContext, path.c_str(), NULL, NULL);
    if (err) {
        av_strerror(err, err_buffer, sizeof(err_buffer));
        ELOG("open input failed|ret:%d|msg:%s", err, err_buffer)
        return;
    }
    err = avformat_find_stream_info(avFormatContext, NULL);
    if (err < 0) {
        ELOG("Could not find stream info")
        return;
    }
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            ILOG("find audio stream index %d", i)
        }
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            ILOG("find video strean index %d", i)
        }
    }
    if (audio_stream_index >= 0) {
        openStream(audio_stream_index);
    }
    if (video_stream_index >= 0) {
        openStream(video_stream_index);
    }
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    if (packet == NULL) {
        ELOG("Could not allocate avPacket")
        return;
    }
    while (true) {
        err = av_read_frame(avFormatContext, packet);
        ILOG("av_read_frame")
        if (err < 0) {
            ELOG("av_read_frame error|ret:%d|msg:%s", err, err_buffer)
            if ((err == AVERROR_EOF) || avio_feof(avFormatContext->pb)) {
                ELOG("av_read_frame eof")
                eof = 1;
                break;
            }
            if (avFormatContext->pb && avFormatContext->pb->error) {
                ELOG("av_read_frame error")
                break;
            }
            av_strerror(err, err_buffer, sizeof(err_buffer));
        }
        if (packet->stream_index == audio_stream_index) {
            audio_packet.Put(packet);
            ILOG("av_read_frame audio packet")
        } else if (packet->stream_index == video_stream_index) {
            video_packet.Put(packet);
            ILOG("av_read_frame video packet")
        } else {
            av_packet_unref(packet);
            ILOG("av_read_frame other packet")
        }
    }
    av_packet_unref(packet);
    av_free_packet(packet);
}

/**
 * 音频解码
 */
void SnailPlayer::decodeAudio() {
    ILOG("SnailPlayer::decodeAudio")
    AVPacket packet;
    AVFrame *frame = av_frame_alloc();
    while (true) {
        audio_packet.Get(&packet);
        int ret;
        ret = avcodec_send_packet(audio_codec_context, &packet);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            ELOG("avcodec_send_packet error|code:%d|msg:%s", ret, GetAVErrorMsg(ret));
            break;
        }
        ret = avcodec_receive_frame(audio_codec_context, frame);
        if (ret < 0 && ret != AVERROR_EOF) {
            ELOG("avcodec_receive_frame error|code:%d|msg:%s", ret, GetAVErrorMsg(ret))
            if (ret == -11) {
                continue;
            }
            break;
        }
        ILOG("Put Audio Packet")
        audio_frame.Put(frame);
        if (audio_frame.Size() >= AUDIO_READY_SIZE && state == State::Preparing) {
            state = State::Prepared;
            if (eventCallback) {
                eventCallback->OnPrepared();
            }
        }
    }
    av_packet_unref(&packet);
    av_free_packet(&packet);
}

/**
 * 视频解码
 */
void SnailPlayer::decodeVideo() {
    ILOG("SnailPlayer::decodeVideo")
    AVPacket packet;
    AVFrame *frame = av_frame_alloc();
    while (true) {
        video_packet.Get(&packet);
        int ret;
        ret = avcodec_send_packet(video_codec_context, &packet);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            ELOG("avcodec_send_packet error|code:%d|msg:%s", ret, GetAVErrorMsg(ret))
            break;
        }
        ret = avcodec_receive_frame(video_codec_context, frame);
        if (ret < 0 && ret != AVERROR_EOF) {
            ELOG("avcodec_receive_frame error|code:%d|msg:%s", ret, GetAVErrorMsg(ret))
            if (ret == -11) {
                continue;
            }
            break;
        }
        ILOG("Put Video Packet")
        video_frame.Put(frame);
    }
    av_packet_unref(&packet);
    av_free_packet(&packet);
}

/**
 * 打开流数据
 * @param index
 */
void SnailPlayer::openStream(int index) {
    ILOG("SnailPlayer::openStream|index:%d", index)
    AVCodecContext *avCodecContext;
    AVCodec *avCodec;
    int ret = 0;
    if (index < 0 || index >= avFormatContext->nb_streams) {
        ELOG("stream index is invalid")
        return;
    }
    avCodecContext = avcodec_alloc_context3(NULL);
    if (!avCodecContext) {
        ELOG("Could not alloc codec context")
        return;
    }
    ret = avcodec_parameters_to_context(avCodecContext, avFormatContext->streams[index]->codecpar);
    if (ret < 0) {
        avcodec_free_context(&avCodecContext);
        ELOG("avcodec_parameters_to_context error %d", ret)
        return;
    }
    av_codec_set_pkt_timebase(avCodecContext, avFormatContext->streams[index]->time_base);
    avCodec = avcodec_find_decoder(avCodecContext->codec_id);
    avCodecContext->codec_id = avCodec->id;
    avFormatContext->streams[index]->discard = AVDISCARD_DEFAULT;
    ret = avcodec_open2(avCodecContext, avCodec, NULL);
    if (ret < 0) {
        ELOG("Fail to open codec on stream:%d|codec:%d", index, ret);
        avcodec_free_context(&avCodecContext);
        return;
    }
    switch (avCodecContext->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            swr_context = swr_alloc();
            swr_context = swr_alloc_set_opts(NULL,
                                             avCodecContext->channel_layout,
                                             AV_SAMPLE_FMT_S16,
                                             avCodecContext->sample_rate,
                                             avCodecContext->channel_layout,
                                             avCodecContext->sample_fmt,
                                             avCodecContext->sample_rate,
                                             0,
                                             NULL);
            if (!swr_context || swr_init(swr_context) < 0) {
                ELOG("Could not create sample rate converterfor conversion channels")
                swr_free(&swr_context);
                return;
            }
            audio_stream = avFormatContext->streams[index];
            audio_codec_context = avCodecContext;
            audio_buffer = (uint8_t *) malloc(sizeof(uint8_t) * AUDIO_BUFFER_SIZE);
            initAudioPlayer(audio_codec_context->sample_rate, audio_codec_context->channels, this);
            audio_decode_thread.reset(new std::thread(&SnailPlayer::decodeAudio, this));
            break;
        case AVMEDIA_TYPE_VIDEO:
            video_stream = avFormatContext->streams[index];
            img_convert_context = sws_getContext(avCodecContext->width, avCodecContext->height,
                                                 avCodecContext->pix_fmt, avCodecContext->width,
                                                 avCodecContext->height, AV_PIX_FMT_RGBA,
                                                 SWS_BICUBIC, NULL, NULL, NULL);
            video_codec_context = avCodecContext;
            frame_rgba = av_frame_alloc();
            int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, avCodecContext->width,
                                                    avCodecContext->height, 1);
            rgba_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
            av_image_fill_arrays(frame_rgba->data, frame_rgba->linesize, rgba_buffer,
                                 AV_PIX_FMT_RGBA, avCodecContext->width, avCodecContext->height, 1);
            video_decode_thread.reset(new std::thread(&SnailPlayer::decodeVideo, this));
            break;
    }
}

/**
 * 开始
 * @return
 */
int SnailPlayer::Start() {
    ILOG("SnailPlayer::Start")
    if (state != State::Prepared) {
        ELOG("illagal state|current:%d", state);
        return ERROR_ILLEGAL_STATE;
    }
    audio_render_thread.reset(new std::thread(startAudioPlay));
    video_render_thread.reset(new std::thread(videoRender));
    state = State::Started;
    return SUCCESS;
}

/**
 * 暂停
 * @return
 */
int SnailPlayer::Pause() {
    ILOG("SnailPlayer::Pause")
    if (state != State::Started) {
        ELOG("illegal state|current:%d", state)
        return ERROR_ILLEGAL_STATE;
    }
    stopAudioPlay();
    state = State::Paused;
    return SUCCESS;
}


/**
 * 继续播放
 * @return
 */
int SnailPlayer::Resume() {
    ILOG("SnailPlayer::Resume")
    if (state != State::Paused) {
        ELOG("illegal state|current:%d", state)
        return ERROR_ILLEGAL_STATE;
    }
    resumeAudioPlay();
    state = State::Started;
    return SUCCESS;
}


/**
 * 获取视频数据
 * @param buffer
 * @param frame
 * @param width
 * @param height
 */
void SnailPlayer::GetData(uint8_t **buffer, AVFrame **frame, int &width, int &height) {
    ILOG("SnailPlayer::GetData Video")
    auto v_frame = video_frame.Get();
    sws_scale(img_convert_context, (const uint8_t *const *) v_frame->data, v_frame->linesize, 0,
              video_codec_context->height, frame_rgba->data, frame_rgba->linesize);
    double timestamp =
            av_frame_get_best_effort_timestamp(v_frame) * av_q2d(video_stream->time_base);
    //由于人对声音的变化比较敏感，所以以音频为基准，控制视频去同步音频
    if (timestamp > audio_clock) {
        ILOG("sleep time = %f", timestamp - audio_clock)
        usleep((unsigned long) ((timestamp - audio_clock) * 1000000));
    }
    ILOG("sleep is finish")
    *frame = frame_rgba;
    *buffer = rgba_buffer;
    width = video_codec_context->width;
    height = video_codec_context->height;

    av_frame_unref(v_frame);
    av_frame_free(&v_frame);
}

/**
 * 获取视频的宽
 * @return
 */
int SnailPlayer::GetVideoWidth() {
    ILOG("SnailPlayer::GetVideoWidth")
    if (video_codec_context) {
        return video_codec_context->width;
    }
    return 0;
}

/**
 * 获取视频的高
 * @return
 */
int SnailPlayer::GetVideoHeight() {
    ILOG("SnailPlayer::GetVideoHeight")
    if (video_codec_context) {
        return video_codec_context->height;
    }
    return 0;
}