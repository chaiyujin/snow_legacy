#include "snow_reader.h"
#include "ffmpeg_functions.h"
#include <memory>
#define SYNC_EPS 5

namespace snow {

bool            MediaReader::gInitialized = false;
snow::color_map MediaReader::gJetCmap = snow::color_map::jet();


MediaReader::MediaReader(const std::string &filename)
    : mFilename(filename)
    , mFmtCtxPtr(nullptr)
    , mErrorAgain(false)
    , mSyncVideoStreams(true)
    , mFPS(0.0)
    , mWavTracks(0) {}

MediaReader::MediaReader(const MediaReader &b) 
    : mFilename(b.mFilename)
    , mStreamPtrList(b.mStreamPtrList)
    , mFmtCtxPtr(b.mFmtCtxPtr)
    , mErrorAgain(b.mErrorAgain)
    , mDstAudioFmt(b.mDstAudioFmt)
    , mSyncVideoStreams(b.mSyncVideoStreams)
    , mVideoQueues(b.mVideoQueues)
    , mAudioQueues(b.mAudioQueues)
    , mFPS(b.mFPS)
    , mWavTracks(b.mWavTracks)
    {}

MediaReader::~MediaReader() {
    close();
}

bool MediaReader::open() {
    if (gInitialized == false) {
        av_log(NULL, AV_LOG_ERROR, "Please initialize ffmpeg first!\n");
        return false;
    }
    int ret;
    if ((ret = avformat_open_input(&mFmtCtxPtr, mFilename.c_str(), NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file %s %s\n", mFilename.c_str(), av_err2str(ret));
        return false;
    }
    if (avformat_find_stream_info(mFmtCtxPtr, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not find stream information\n");
        close();
        return false;
    }

    // not seek timestamp
    int64_t timestamp = 0;
    if (mFmtCtxPtr->start_time != AV_NOPTS_VALUE)
        timestamp += mFmtCtxPtr->start_time;

    /* -- open streams -- */
    int video_streams = 0;
    int audio_streams = 0;
    for (uint32_t i = 0; i < mFmtCtxPtr->nb_streams; i++) {
        AVStream *stream = mFmtCtxPtr->streams[i];
        AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
        AVCodecContext *codec_ctx;
        if (!dec) {
            av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
            close();
            return false;
        }
        codec_ctx = avcodec_alloc_context3(dec);
        if (!codec_ctx) {
            av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
            close();
            return false;
        }
        ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
                "for stream #%u\n", i);
            close();
            return false;
        }

        /* Reencode video & audio and remux subtitles etc. */
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                codec_ctx->framerate = av_guess_frame_rate(mFmtCtxPtr, stream, NULL);
                double fps = (double)codec_ctx->framerate.num / (double)codec_ctx->framerate.den;
                if (mFPS == 0.0) mFPS = fps;
                else if (abs(mFPS - fps) > 1e-6) {
                    av_log(NULL, AV_LOG_ERROR, "Video tracks have different fps %f %f\n", mFPS, fps);
                    close();
                    return false;
                }
            }
            /* Open decoder */
            ret = avcodec_open2(codec_ctx, dec, NULL);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
                close();
                return ret;
            }
        }
        InputStream *st = new InputStream;
        st->mType       = codec_ctx->codec_type;
        st->mStreamPtr  = stream;
        st->mDecPtr     = dec;
        st->mDecCtxPtr  = codec_ctx;
        st->mNumSamples = 0;
        st->mMinPts     = INT64_MAX;
        st->mMaxPts     = INT64_MIN;
        if (st->mType == AVMEDIA_TYPE_AUDIO) {

            st->mAudioStreamIdx = audio_streams++;
            mAudioQueues.push_back(new SafeQueue<AudioFrame>());

            int     srcNumSamples = 1024;
            int64_t dstNumSample = av_rescale_rnd(srcNumSamples, mDstAudioFmt.mSampleRate, codec_ctx->sample_rate, AV_ROUND_UP);

            st->mTmpFramePtr = ffmpeg::allocAudioFrame(
                mDstAudioFmt.mSampleFmt,
                mDstAudioFmt.mChLayout,
                mDstAudioFmt.mSampleRate,
                dstNumSample);
            {
                st->mSwrCtxPtr = swr_alloc();
                if (!st->mSwrCtxPtr) {
                    fprintf(stderr, "Could not allocate resampler context\n");
                    exit(1);
                }

                /* set options */
                av_opt_set_int(st->mSwrCtxPtr,        "in_channel_layout",  codec_ctx->channel_layout,  0);
                av_opt_set_int(st->mSwrCtxPtr,        "in_channel_count",   codec_ctx->channels,        0);
                av_opt_set_int(st->mSwrCtxPtr,        "in_sample_rate",     codec_ctx->sample_rate,     0);
                av_opt_set_sample_fmt(st->mSwrCtxPtr, "in_sample_fmt",      codec_ctx->sample_fmt,      0);
                av_opt_set_int(st->mSwrCtxPtr,        "out_channel_layout", mDstAudioFmt.mChLayout,     0);
                av_opt_set_int(st->mSwrCtxPtr,        "out_channel_count",  mDstAudioFmt.mChannels,     0);
                av_opt_set_int(st->mSwrCtxPtr,        "out_sample_rate",    mDstAudioFmt.mSampleRate,   0);
                av_opt_set_sample_fmt(st->mSwrCtxPtr, "out_sample_fmt",     mDstAudioFmt.mSampleFmt,    0);

                /* initialize the resampling context */
                if ((ret = swr_init(st->mSwrCtxPtr)) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to initialize the resampling context\n");
                    exit(1);
                }
            }
        }
        else if (st->mType == AVMEDIA_TYPE_VIDEO)
        {
            st->mVideoStreamIdx = video_streams++;
            mVideoQueues.push_back(new SafeQueue<VideoFrame>());
            // if (st->is_depth()) { _depth_indices.push_back(st->mVideoStreamIdx); }

            st->mTmpFramePtr = ffmpeg::allocPicture(AV_PIX_FMT_RGBA, codec_ctx->width, codec_ctx->height);
            if (codec_ctx->pix_fmt != AV_PIX_FMT_RGBA && !st->is_depth()) {
                st->mSwsCtxPtr = sws_getContext(
                    codec_ctx->width, codec_ctx->height,
                    codec_ctx->pix_fmt,
                    codec_ctx->width, codec_ctx->height,
                    AV_PIX_FMT_RGBA,
                    SWS_BICUBIC, NULL, NULL, NULL);
                if (!st->mSwsCtxPtr)
                {
                    av_log(NULL, AV_LOG_ERROR, "Could not initialize the conversion context\n");
                }
            }
        }
        mStreamPtrList.push_back(st);
    }
    // av_dump_format(mFmtCtxPtr, 0, mFilename.c_str(), 0);

    mStartTime = 0;
    mDuration  = durationMs();

    // pre read audio tracks
    preReadAudioTracks();

    return true;
}

void MediaReader::close() {
    clearQueues();
    for (auto *st : mStreamPtrList) delete st;
    mStreamPtrList.clear();
    if (mFmtCtxPtr) {
        std::lock_guard<std::mutex> lock(mFmtCtxMutex);
        avformat_close_input(&mFmtCtxPtr);
        mFmtCtxPtr = nullptr;
    }
}


void MediaReader::clearQueues() {
    for (auto *q : mVideoQueues) q->clear();
    for (auto *q: mAudioQueues)  q->clear();
}

int MediaReader::processInput(MediaType request_type) {
    std::lock_guard<std::mutex> lock(mFmtCtxMutex);
    if (!mFmtCtxPtr) return AVERROR_EOF;

    int ret;
    AVPacket pkt;
    AVFrame *frame;
    /* read frame */
    ret = av_read_frame(mFmtCtxPtr, &pkt);
    if (ret == AVERROR(EAGAIN)) {
        mErrorAgain = true;
        return ret;
    }
    if (ret == AVERROR_EOF) {
        // push null frame to tell it's over
        for (int i = 0; i < mVideoQueues.size(); ++i)
            mVideoQueues[i]->push(VideoFrame());
        for (int i = 0; i < mAudioQueues.size(); ++i)
            mAudioQueues[i]->push(AudioFrame());
        return ret;
    }

    InputStream *st = mStreamPtrList[pkt.stream_index];

    int got_frame = 0;
    if ((st->mType == AVMEDIA_TYPE_AUDIO) && (request_type & MediaType::Audio))
    {
        ffmpeg::decode(st->mDecCtxPtr, st->mDecodeFramePtr, &got_frame, &pkt);
        if (got_frame) {
            int64_t pts = av_frame_get_best_effort_timestamp(st->mDecodeFramePtr);
#ifdef DEBUG_TS
            std::cout << pkt.stream_index << " " << pts << std::endl;
            printf("%s %d: pts %f nb_samples %d %s\n",
                st->dec->name, pkt.stream_index,
                pts, st->mDecodeFramePtr->nb_samples,
                av_get_sample_fmt_name(st->mDecCtxPtr->sample_fmt));
#endif
            int nb_samples = 0;
            /* resample */ {
                // compute the converted samples number
                int dstNumSample = av_rescale_rnd(
                    swr_get_delay(st->mSwrCtxPtr, st->mDecCtxPtr->sample_rate) + st->mDecodeFramePtr->nb_samples,
                    mDstAudioFmt.mSampleRate, st->mDecCtxPtr->sample_rate, AV_ROUND_UP);
                // realloc the temporary frame
                if (st->mTmpFramePtr->nb_samples < dstNumSample) {
                    av_frame_free(&st->mTmpFramePtr);
                    st->mTmpFramePtr = ffmpeg::allocAudioFrame(
                        mDstAudioFmt.mSampleFmt, mDstAudioFmt.mChLayout,
                        mDstAudioFmt.mSampleRate, dstNumSample);
                }
                // software resample
                nb_samples = swr_convert(
                    st->mSwrCtxPtr, st->mTmpFramePtr->data, dstNumSample,
                    (const uint8_t **)st->mDecodeFramePtr->data, st->mDecodeFramePtr->nb_samples);
            }
            /* send to queue */ {
                AudioFrame audio_frame;
                audio_frame.fromAVFrame(st->mTmpFramePtr, pts, nb_samples, 2);
                mAudioQueues[st->mAudioStreamIdx]->push(audio_frame);
            }
        }
    }
    else if ((st->mType == AVMEDIA_TYPE_VIDEO) && (request_type & MediaType::Video)) {
        ffmpeg::decode(st->mDecCtxPtr, st->mDecodeFramePtr, &got_frame, &pkt);
        if (got_frame) {
            int64_t pts = av_frame_get_best_effort_timestamp(st->mDecodeFramePtr);
#ifdef DEBUG_TS
            std::cout << pkt.stream_index << " " << pts << std::endl;
            printf("%s %d: pts %f %s\n",
                st->dec->name, pkt.stream_index,
                pts, av_get_pix_fmt_name(st->mDecCtxPtr->pix_fmt));
#endif
            frame = st->mDecodeFramePtr;
            if (!st->is_depth() && st->mSwsCtxPtr) {
                sws_scale(st->mSwsCtxPtr, (const uint8_t * const *)st->mDecodeFramePtr->data,
                          st->mDecodeFramePtr->linesize, 0, st->mDecodeFramePtr->height,
                          st->mTmpFramePtr->data, st->mTmpFramePtr->linesize);
                frame = st->mTmpFramePtr;
            }
            // send to queue
            {
                VideoFrame video_frame;
                video_frame.fromAVFrame(frame, pts, st->is_depth());
                mVideoQueues[st->mVideoStreamIdx]->push(video_frame);
            }
        }
    }

    if (got_frame) {
        av_packet_unref(&pkt);
    }

    return ret;
}

void MediaReader::syncVideoStreams() {
    auto quitEOF = [this]() -> void {
        for (auto * q : mVideoQueues) {
            q->clear();
            q->push(VideoFrame());
        }
    };

    // 1. read frames & get max pts
    int64_t max_pts = -1000.0;
    int count = 0;
    do {
        for (int i = 0; i < mVideoQueues.size(); ++i) {
            if (mVideoQueues[i]->size()) {
                ++count;
                max_pts = std::max(max_pts, mVideoQueues[i]->front().timestamp());
            }
        }
        // all queues have a frame
        if (count == (int)mVideoQueues.size()) break;
        // need to read frame from video
        if (processInput() == AVERROR_EOF) { quitEOF(); return; } // end of file return null pair.
        count = 0;
    } while (count == 0);

    // 2. get frame pair
    for (auto *q : mVideoQueues) {
        while (abs(q->front().timestamp() - max_pts) > SYNC_EPS && q->front().timestamp() < max_pts) {
            q->pop();
            while (q->size() == 0)
                if (processInput() == AVERROR_EOF) { quitEOF(); return; }
        }
    }

    // // now each video queue has at least one frame, they are all at the close ts.
    // printf("=====\n");
    // for (size_t i = 0; i < mVideoQueues.size(); ++i) {
    //     auto *q = mVideoQueues[i];
    //     printf(" video stream %d: timestamp %d ms.\n", i, q->front().timestamp());
    // }
    // printf("=====\n");
}

int64_t MediaReader::durationMs() {
    return (mFmtCtxPtr)? av_rescale_q(mFmtCtxPtr->duration, AV_TIME_BASE_Q, AVRational{ 1, 1000 }) : 0;
}

void MediaReader::seek(int64_t ms) {
    if (mFmtCtxPtr) {
        clearQueues();
        std::lock_guard<std::mutex> lock(mFmtCtxMutex);       
        int64_t ts = av_rescale_q(ms, AVRational{ 1, 1000 }, AV_TIME_BASE_Q);
        av_seek_frame(mFmtCtxPtr, -1, ts, AVSEEK_FLAG_BACKWARD);
    }
}

/**
 * Video: 1. all video streams will be synced. 2. just read
 * Audio: just read.
 * */
std::unique_ptr<FrameBase> MediaReader::readFrame(const StreamBase *st) {
    const auto *stream = (const MediaStream *)st;
    if (stream->type() == MediaType::Video) {
        int index = stream->streamIndex();
        if (mSyncVideoStreams) {
            while (mVideoQueues[index]->size() == 0)   syncVideoStreams();
            if (mVideoQueues[index]->front().isNull()) return nullptr;
        }
        else
            while (mVideoQueues[index]->size() == 0)   if (processInput() == AVERROR_EOF) return nullptr;
        return std::unique_ptr<FrameBase> (new VideoFrame(mVideoQueues[index]->frontAndPop()));
    }
    else if (stream->type() == MediaType::Audio) {
        int index = stream->streamIndex();
        // normally read
        while (mAudioQueues[index]->size() == 0)
            if (processInput() == AVERROR_EOF) return nullptr;
        return std::unique_ptr<FrameBase> (new AudioFrame(mAudioQueues[index]->frontAndPop()));
    }
    else {
        throw std::runtime_error("[MediaReader]: only support Video and Audio stream.");
    }
}

std::vector<std::shared_ptr<StreamBase>> MediaReader::getStreams() {
    std::vector<std::shared_ptr<StreamBase>> ret;
    int64_t duration = durationMs();
    for (size_t i = 0; i < mStreamPtrList.size(); ++i) {
        auto * st = mStreamPtrList[i];
        MediaType type  = (st->mType == AVMEDIA_TYPE_VIDEO) ? MediaType::Video : MediaType::Audio;
        int streamIndex = (st->mType == AVMEDIA_TYPE_VIDEO) ? st->mVideoStreamIdx : st->mAudioStreamIdx;
        VideoFormat videoFmt;
        AudioFormat audioFmt;
        if (st->mType == AVMEDIA_TYPE_VIDEO) {
            videoFmt.mWidth = st->mDecCtxPtr->width;
            videoFmt.mHeight = st->mDecCtxPtr->height;
            videoFmt.mPixelFmt = st->mDecCtxPtr->pix_fmt;
            videoFmt.mIsDepth = st->is_depth();
        }
        else
            audioFmt = mDstAudioFmt;
        ret.emplace_back(new MediaStream(streamIndex, type, 0, duration, this, audioFmt, videoFmt));
    }
    return ret;
}

void MediaReader::preReadAudioTracks() {
    seek(0);
    mWavTracks.clear();

    while (processInput(MediaType::Audio) !=  AVERROR_EOF) ;
    int iTrack = 0;
    for (auto *q : mAudioQueues) {
        bool invalid = false;
        std::vector<float> track;
        int64_t startTime = q->front().timestamp();
        // padding zero for starttime
        const int padding = startTime * mDstAudioFmt.mSampleRate / 1000;
        startTime -= padding * 1000 / mDstAudioFmt.mSampleRate;
        for (int i = 0; i < padding; ++i) track.push_back(0.0f);
        // read from frames
        while (q->size()) {
            AudioFrame frame = q->frontAndPop();
            if (frame.isNull()) break;
            if (frame.mBytePerSample == 1) {
                uint8_t val;
                const uint8_t *data = frame.data();
                for (int i = 0; i < frame.mNumSamples; ++i) {
                    val = data[i];
                    track.push_back( ((float)val - 128.0f) / 128.0f );
                }
            }
            else if (frame.mBytePerSample == 2) {
                int16_t val;
                const int16_t * data = (const int16_t *)frame.data();
                for (int i = 0; i < frame.mNumSamples; ++i) {
                    val = data[i];
                    track.push_back( (float)val / 32767.0f );
                }
            }
            else {
                printf("only support uint8_t and int16_t audio, not bytes %d!\n", frame.mBytePerSample);
                invalid = true;
                break;
            }
        }
        if (!invalid) {
            mWavTracks.emplace_back();
            mWavTracks.back().setSampleRate(mDstAudioFmt.mSampleRate);
            mWavTracks.back().addChannel(track);
            mWavTracks.back().setStartTime(startTime);
            // mWavTracks.back().write(std::string("../../../assets/test") + std::to_string(iTrack) + ".wav");
            // printf("[MediaReader]: Audio track %d: start at %d, has %d samples, %d sr\n",
            //        iTrack, startTime, track.size(), mDstAudioFmt.mSampleRate);
        }
        ++iTrack;
    }

    seek(0);
}

}