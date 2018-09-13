#include "snow_vreader.h"

#define SYNC_EPS 5

namespace snow {

bool            DepthVideoReader::gInitialized = false;
snow::color_map DepthVideoReader::gJetCmap = snow::color_map::jet();

AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                           uint64_t channel_layout,
                           int sample_rate,
                           int64_t nb_samples) {
    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }

    return frame;
}

AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height) {
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;
}

DepthVideoReader::DepthVideoReader(const std::string &filename)
    : mFilename(filename)
    , mFmtCtxPtr(nullptr)
    , mErrorAgain(false) {}

DepthVideoReader::DepthVideoReader(const DepthVideoReader &b) 
    : mFilename(b.mFilename)
    , mStreamPtrList(b.mStreamPtrList)
    , mFmtCtxPtr(b.mFmtCtxPtr)
    , mStartTime(b.mStartTime)
    , mInputTsOffset(b.mInputTsOffset)
    , mTsOffset(b.mTsOffset)
    , mDuration(b.mDuration)
    , mTimeBase(b.mTimeBase)
    , mErrorAgain(b.mErrorAgain)
    , mDstAudioFmt(b.mDstAudioFmt)
    , mVideoQueues(b.mVideoQueues)
    , mAudioQueues(b.mAudioQueues)
    {}

DepthVideoReader::~DepthVideoReader() {
    close();
}

bool DepthVideoReader::open() {
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
            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
                codec_ctx->framerate = av_guess_frame_rate(mFmtCtxPtr, stream, NULL);
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
            // get sampel rate
            mDstAudioFmt.mSampleRate = codec_ctx->sample_rate;

            st->mAudioStreamIdx = audio_streams++;
            mAudioQueues.push_back(new SafeQueue<AudioFrame>());

            int     src_nb_samples = (codec_ctx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) ? 10000 : codec_ctx->frame_size;
            int64_t dst_nb_samples = av_rescale_rnd(src_nb_samples, mDstAudioFmt.mSampleRate, codec_ctx->sample_rate, AV_ROUND_UP);

            st->mTmpFramePtr = alloc_audio_frame(
                mDstAudioFmt.mSampleFmt,
                mDstAudioFmt.mChLayout,
                mDstAudioFmt.mSampleRate,
                dst_nb_samples);
            {
                st->mSwrCtxPtr = swr_alloc();
                if (!st->mSwrCtxPtr) {
                    fprintf(stderr, "Could not allocate resampler context\n");
                    exit(1);
                }

                /* set options */
                av_opt_set_int(st->mSwrCtxPtr, "in_channel_count", codec_ctx->channels, 0);
                av_opt_set_int(st->mSwrCtxPtr, "in_sample_rate", codec_ctx->sample_rate, 0);
                av_opt_set_sample_fmt(st->mSwrCtxPtr, "in_sample_fmt", codec_ctx->sample_fmt, 0);
                av_opt_set_int(st->mSwrCtxPtr, "out_channel_count", mDstAudioFmt.mChannels, 0);
                av_opt_set_int(st->mSwrCtxPtr, "out_sample_rate", mDstAudioFmt.mSampleRate, 0);
                av_opt_set_sample_fmt(st->mSwrCtxPtr, "out_sample_fmt", mDstAudioFmt.mSampleFmt, 0);

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

            st->mTmpFramePtr = alloc_picture(AV_PIX_FMT_RGBA, codec_ctx->width, codec_ctx->height);
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

    mStartTime = AV_NOPTS_VALUE;
    mInputTsOffset = 0;
    mTsOffset = (0 - timestamp);   // not copy ts
    mDuration = 0;
    mTimeBase = AVRational{ 1, 1 };

    return true;
}

void DepthVideoReader::close() {
    for (auto *st : mStreamPtrList) delete st;
    mStreamPtrList.clear();
    if (mFmtCtxPtr) {
        std::lock_guard<std::mutex> lock(mFmtCtxMutex);
        avformat_close_input(&mFmtCtxPtr);
        mFmtCtxPtr = nullptr;
    }
}

static int decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt) {
    int ret;
    int consumed = 0;

    *got_frame = 0;

    // This relies on the fact that the decoder will not buffer additional
    // packets internally, but returns AVERROR(EAGAIN) if there are still
    // decoded frames to be returned.
    ret = avcodec_send_packet(avctx, pkt);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        return ret;
    if (ret >= 0)
        consumed = pkt->size;

    ret = avcodec_receive_frame(avctx, frame);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        return ret;
    if (ret >= 0)
        *got_frame = 1;

    return consumed;
}


int DepthVideoReader::process_input(MediaType request_type) {
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
        decode(st->mDecCtxPtr, st->mDecodeFramePtr, &got_frame, &pkt);
        if (got_frame) {
            int64_t pts = av_frame_get_best_effort_timestamp(st->mDecodeFramePtr);
#ifdef DEBUG_TS
            std::cout << pkt.stream_index << " " << pts << std::endl;
            printf("%s %d: pts %f nb_samples %d %s\n",
                st->dec->name, pkt.stream_index,
                pts, st->mDecodeFramePtr->nb_samples,
                av_get_sample_fmt_name(st->mDecCtxPtr->sample_fmt));
#endif
            // compute the converted samples number
            int dst_nb_samples = av_rescale_rnd(
                swr_get_delay(st->mSwrCtxPtr, st->mDecCtxPtr->sample_rate) + st->mDecodeFramePtr->nb_samples,
                mDstAudioFmt.mSampleRate, st->mDecCtxPtr->sample_rate, AV_ROUND_UP);
            // realloc the temporary frame
            if (st->mTmpFramePtr->nb_samples < dst_nb_samples) {
                av_frame_free(&st->mTmpFramePtr);
                st->mTmpFramePtr = alloc_audio_frame(
                    mDstAudioFmt.mSampleFmt,
                    mDstAudioFmt.mChLayout,
                    mDstAudioFmt.mSampleRate,
                    dst_nb_samples);
            }
            // software resample
            swr_convert(st->mSwrCtxPtr, st->mTmpFramePtr->data, dst_nb_samples,
                        (const uint8_t **)st->mDecodeFramePtr->data, st->mDecodeFramePtr->nb_samples);
            // send to queue
            {
                AudioFrame audio_frame;
                audio_frame.fromAVFrame(st->mTmpFramePtr, pts, dst_nb_samples, 2);
                mAudioQueues[st->mAudioStreamIdx]->push(audio_frame);
            }
        }
    }
    else if ((st->mType == AVMEDIA_TYPE_VIDEO) && (request_type & MediaType::Video)) {
        decode(st->mDecCtxPtr, st->mDecodeFramePtr, &got_frame, &pkt);
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

std::pair<VideoFrame, VideoFrame> DepthVideoReader::read_frame_pair() {
    std::pair<VideoFrame, VideoFrame> ret;
    // 1. read frames & get max pts
    int64_t max_pts = -1000.0;
    int count = 0;
    do {
        for (int i = 0; i < mVideoQueues.size(); ++i) {
            if (mVideoQueues[i]->size()) {
                ++count;
                max_pts = std::max(max_pts, mVideoQueues[i]->front().mTimeStamp);
            }
        }
        // all queues have a frame
        if (count == (int)mVideoQueues.size()) break;
        // need to read frame from video
        if (process_input() == AVERROR_EOF) return ret; // end of file return null pair.
        count = 0;
    } while (count == 0);

    // 2. get frame pair
    for (auto *q : mVideoQueues) {
        while (abs(q->front().mTimeStamp - max_pts) > SYNC_EPS && q->front().mTimeStamp < max_pts) {
            q->pop();
            while (q->size() == 0)
                if (process_input() == AVERROR_EOF) return ret;
        }
        if (q->front().mIsDepth)
            ret.second = q->front();
        else ret.first = q->front();
        q->pop();
    }
    return ret;
}

int64_t DepthVideoReader::duration_ms() {
    std::lock_guard<std::mutex> lock(mFmtCtxMutex); 
    return (mFmtCtxPtr)? av_rescale_q(mFmtCtxPtr->duration, AV_TIME_BASE_Q, AVRational{ 1, 1000 }) : 0;
}

void DepthVideoReader::seek(int64_t ms) {
    if (mFmtCtxPtr) {
        std::lock_guard<std::mutex> lock(mFmtCtxMutex);       
        int64_t ts = av_rescale_q(ms, AVRational{ 1, 1000 }, AV_TIME_BASE_Q);
        int ret = av_seek_frame(mFmtCtxPtr, -1, ts, AVSEEK_FLAG_BACKWARD);
    }
}

FrameBase * DepthVideoReader::operator()(int64_t id, MediaType type) {
    while (mVideoQueues[id]->size() == 0)
        if (process_input() == AVERROR_EOF) return nullptr;
    VideoFrame * frame = new VideoFrame(mVideoQueues[id]->front());
    return frame;
}

}