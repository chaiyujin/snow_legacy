#include "snow_writer.h"
#include <string>
#include <stdexcept>

namespace snow {
static AVFrame *allocAudioFrame(enum AVSampleFormat sampleFmt, uint64_t channelLayout, int sampleRate, int nbSamples) {
    AVFrame *frame = av_frame_alloc();
    if (!frame) { av_log(nullptr, AV_LOG_ERROR, "Error allocating an audio frame\n"); exit(1); }
    frame->format           = sampleFmt;
    frame->channel_layout   = channelLayout;
    frame->sample_rate      = sampleRate;
    frame->nb_samples       = nbSamples;
    if (nbSamples) {
        int ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Error allocating an audio buffer\n"); exit(1); }
    }
    return frame;
}

static AVFrame *allocPicture(enum AVPixelFormat pixFmt, int width, int height) {
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture) return nullptr;

    picture->format = pixFmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Could not allocate frame data.\n"); exit(1); }

    return picture;
}

static int writeFrame(AVFormatContext *fmtCtx, const AVRational *timeBase, AVStream *st, AVPacket *pkt) {
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, *timeBase, st->time_base);
    pkt->stream_index = st->index;
    /* Write the compressed frame to the media file. */
    return av_interleaved_write_frame(fmtCtx, pkt);
}

static OutputStream * addStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codecId, int width, int height, int fps, int sampleRate) {
    OutputStream *ost = new OutputStream;

    *codec = avcodec_find_encoder(codecId);
    if (!(*codec)) {
        av_log(nullptr, AV_LOG_ERROR, "Could not find encoder for '%s'\n", avcodec_get_name(codecId));
        exit(1);
    }
    ost->mStreamPtr = avformat_new_stream(oc, nullptr);
    if (!ost->mStreamPtr) {
        av_log(nullptr, AV_LOG_ERROR, "Could not allocate stream\n");
        exit(1);
    }
    ost->mStreamPtr->id = oc->nb_streams-1;
    AVCodecContext *c = avcodec_alloc_context3(*codec);
    if (!c) {
        av_log(nullptr, AV_LOG_ERROR, "Could not alloc an encoding context\n");
        exit(1);
    }
    ost->mEncCtxPtr = c;

    switch ((*codec)->type) {
    case AVMEDIA_TYPE_AUDIO:
        c->sample_fmt  = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        c->bit_rate    = 64000;
        c->sample_rate = sampleRate;
        if ((*codec)->supported_samplerates) {
            c->sample_rate = (*codec)->supported_samplerates[0];
            for (int i = 0; (*codec)->supported_samplerates[i]; i++) {
                if ((*codec)->supported_samplerates[i] == sampleRate)
                    c->sample_rate = sampleRate;
            }
        }
        c->channel_layout = AV_CH_LAYOUT_MONO;
        if ((*codec)->channel_layouts) {
            c->channel_layout = (*codec)->channel_layouts[0];
            for (int i = 0; (*codec)->channel_layouts[i]; i++) {
                if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_MONO)
                    c->channel_layout = AV_CH_LAYOUT_MONO;
            }
        }
        c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
        ost->mStreamPtr->time_base = (AVRational){ 1, c->sample_rate };
        break;

    case AVMEDIA_TYPE_VIDEO:
        c->codec_id = codecId;

        c->bit_rate = 400000;
        /* Resolution must be a multiple of two. */
        c->width    = width;
        c->height   = height;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
        ost->mStreamPtr->time_base = (AVRational){ 1, fps };
        c->time_base       = ost->mStreamPtr->time_base;

        c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt       = AV_PIX_FMT_YUV420P;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B-frames */
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
        }
    break;

    default:
        break;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    return ost;
}

MediaWriter::MediaWriter(std::string filename)
    : mFilename(filename)
    , mFmtCtxPtr(nullptr)
    , mAudioCodecPtr(nullptr)
    , mVideoCodecPtr(nullptr)
    , mAudioStreamPtr(nullptr)
    , mVideoStreamPtr(nullptr)
    , mEncodeVideo(false)
    , mEncodeAudio(false) {
    
    av_register_all();

    /* allocate the output media context */
    avformat_alloc_output_context2(&mFmtCtxPtr, nullptr, nullptr, filename.c_str());
    if (!mFmtCtxPtr) {
        av_log(nullptr, AV_LOG_WARNING, "Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&mFmtCtxPtr, nullptr, "mpeg", filename.c_str());
    }
    if (!mFmtCtxPtr) { throw std::runtime_error("could not alloc output context"); }
}

void MediaWriter::addAudioStream(int sampleRate) {
    if (mAudioStreamPtr) { throw std::runtime_error("[MediaWriter]: you have set audio stream already."); }
    if (mFmtCtxPtr->oformat->audio_codec != AV_CODEC_ID_NONE) {
        mAudioStreamPtr = addStream(mFmtCtxPtr, &mAudioCodecPtr, mFmtCtxPtr->oformat->audio_codec, 0, 0, 0, sampleRate);
    }
    mSampleIndex = 0;
    openAudio();
    mEncodeAudio = true;
}

void MediaWriter::addVideoStream(int w, int h, int bpp, int fps) {
    if (mVideoStreamPtr) { throw std::runtime_error("[MediaWriter]: you have set video stream already."); }
    if (mFmtCtxPtr->oformat->video_codec != AV_CODEC_ID_NONE) {
        mVideoStreamPtr = addStream(mFmtCtxPtr, &mVideoCodecPtr, mFmtCtxPtr->oformat->video_codec, w, h, fps, 0);
    }
    openVideo(bpp);
    mEncodeVideo = true;
}

void MediaWriter::setAudioData(const std::vector<float> &audio) {
    mAudio = audio;
}

void MediaWriter::setAudioData(const std::vector<int16_t> &audio) {
    mAudio.resize(audio.size());
    for (size_t i = 0; i < mAudio.size(); ++i) mAudio[i] = ((float)audio[i] / 32767.0);
}

void MediaWriter::openAudio() {
    AVFormatContext *oc = mFmtCtxPtr;
    AVCodec *codec      = mAudioCodecPtr;
    OutputStream *ost   = mAudioStreamPtr;
    AVCodecContext *c   = ost->mEncCtxPtr;
    int ret;
    /* open it */
    ret = avcodec_open2(c, codec, nullptr);
    if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Could not open audio codec: %s\n", av_err2str(ret)); exit(1); }

    int nbSamples = (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) ? 10000 : c->frame_size;
    ost->mFramePtr = allocAudioFrame(c->sample_fmt, c->channel_layout, c->sample_rate, nbSamples);

    ret = avcodec_parameters_from_context(ost->mStreamPtr->codecpar, c);
    if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Could not copy the stream parameters\n"); exit(1); }
}

void MediaWriter::openVideo(int bpp) {
    AVFormatContext *oc = mFmtCtxPtr;
    AVCodec *codec      = mVideoCodecPtr;
    OutputStream *ost   = mVideoStreamPtr;
    AVCodecContext *c   = ost->mEncCtxPtr;
    int ret;

    /* open the codec */
    ret = avcodec_open2(c, codec, nullptr);
    if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Could not open video codec: %s\n", av_err2str(ret)); exit(1); }

    /* allocate and init a re-usable frame */
    ost->mFramePtr = allocPicture(c->pix_fmt, c->width, c->height);
    if (!ost->mFramePtr) { av_log(nullptr, AV_LOG_ERROR, "Could not allocate video frame\n"); exit(1); }

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    AVPixelFormat pixFmt;
    if (bpp == 3) pixFmt = AV_PIX_FMT_RGB24;
    else if (bpp == 4) pixFmt = AV_PIX_FMT_RGBA;
    else throw std::runtime_error("[MediaWriter]: image bpp is not 3 or 4.");
    ost->mTmpFramePtr = allocPicture(pixFmt, c->width, c->height);
    if (!ost->mTmpFramePtr) { av_log(nullptr, AV_LOG_ERROR, "Could not allocate temporary picture\n"); exit(1); }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->mStreamPtr->codecpar, c);
    if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Could not copy the stream parameters\n"); exit(1); }
}

bool MediaWriter::writeAudioFrame() {
    AVCodecContext *c;
    AVPacket pkt = { 0 }; // data and size must be 0;
    AVFrame *frame = mAudioStreamPtr->mFramePtr;
    int ret;
    int gotPacket;
    int numSamples;

    av_init_packet(&pkt);
    c = mAudioStreamPtr->mEncCtxPtr;

    // get frame
    numSamples = std::min((int)mAudio.size() - mSampleIndex, frame->nb_samples);
    if (numSamples > 0) {
        float *q = (float*)frame->data[0];
        for (int i = 0; i < numSamples; ++i) {
            *q++ = mAudio[mSampleIndex++];
        }
        frame->nb_samples = numSamples;
        frame->pts = mAudioStreamPtr->mNextPts;
        mAudioStreamPtr->mNextPts += frame->nb_samples;
    }
    else frame = nullptr;

    // encode
    ret = avcodec_encode_audio2(c, &pkt, frame, &gotPacket);
    if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Error encoding audio frame: %s\n", av_err2str(ret)); exit(1); }
    if (gotPacket) {
        ret = writeFrame(mFmtCtxPtr, &c->time_base, mAudioStreamPtr->mStreamPtr, &pkt);
        if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Error while writing audio frame: %s\n", av_err2str(ret)); exit(1); }
    }
    // if have new frame, or encoder got packet
    return (frame || gotPacket) ? true : false;
}

bool MediaWriter::writeVideoFrame(const snow::Image *image) {

    AVCodecContext *c;
    AVPacket pkt = { 0 }; // data and size must be 0;
    AVFrame *frame;
    int ret;
    int gotPacket = 0;

    av_init_packet(&pkt);
    c = mAudioStreamPtr->mEncCtxPtr;

    if (!image) frame = nullptr;
    else {
        OutputStream *ost = mVideoStreamPtr;
        // convert image
        if (av_frame_make_writable(ost->mFramePtr) < 0) exit(1);

        if (!ost->mSwsCtxPtr) {
            ost->mSwsCtxPtr = sws_getContext(c->width, c->height,
                                          AV_PIX_FMT_YUV420P,
                                          c->width, c->height,
                                          c->pix_fmt,
                                          SWS_BICUBIC, NULL, NULL, NULL);
            if (!ost->mSwsCtxPtr) { av_log(nullptr, AV_LOG_ERROR, "Could not initialize the conversion context\n"); exit(1); }
        }

        // copy data !!!!!
        -----------------

        sws_scale(ost->mSwsCtxPtr,
                  (const uint8_t * const *)ost->mTmpFramePtr->data, ost->mTmpFramePtr->linesize,
                  0, c->height, ost->mFramePtr->data, ost->mFramePtr->linesize);
    }

    /* encode the image */
    ret = avcodec_encode_video2(c, &pkt, frame, &gotPacket);
    if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Error encoding video frame: %s\n", av_err2str(ret)); exit(1); }
    if (gotPacket) { ret = writeFrame(mFmtCtxPtr, &c->time_base, mVideoStreamPtr->mStreamPtr, &pkt); } else { ret = 0; }
    if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Error while writing video frame: %s\n", av_err2str(ret)); exit(1); }

    return (frame || gotPacket) ? true : false;
}

void MediaWriter::appendImage(const snow::Image &image) {
    if (mVideoStreamPtr == nullptr) { throw std::runtime_error("[MediaWriter]: no video stream added!"); }
    if (mAudioStreamPtr) {
        while (mEncodeAudio) {
            bool videoTurn = av_compare_ts(mVideoStreamPtr->mNextPts, mVideoStreamPtr->mEncCtxPtr->time_base,
                                           mAudioStreamPtr->mNextPts, mAudioStreamPtr->mEncCtxPtr->time_base) <= 0;
            if (videoTurn) break;
            mEncodeAudio = writeAudioFrame();
        }
    }
    mEncodeVideo = writeVideoFrame(&image);
}

void MediaWriter::start(bool dumpFormat) {
    if (dumpFormat) av_dump_format(mFmtCtxPtr, 0, mFilename.c_str(), 1);

    if (!(mFmtCtxPtr->oformat->flags & AVFMT_NOFILE)) {
        int ret = avio_open(&mFmtCtxPtr->pb, mFilename.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Could not open '%s': %s\n", mFilename.c_str(), av_err2str(ret)); exit(1); }
    }

    /* Write the stream header, if any. */
    int ret = avformat_write_header(mFmtCtxPtr, nullptr);
    if (ret < 0) { av_log(nullptr, AV_LOG_ERROR, "Error occurred when opening output file: %s\n", av_err2str(ret)); exit(1); }

    // for audio sample indexing
    mSampleIndex = 0;
}

void MediaWriter::write() {

}

void MediaWriter::finish() {
    // empty all data
    while (mEncodeAudio || mEncodeVideo) {
        if (mEncodeVideo &&
            (!mEncodeAudio || av_compare_ts(mVideoStreamPtr->mNextPts, mVideoStreamPtr->mEncCtxPtr->time_base,
                                            mAudioStreamPtr->mNextPts, mAudioStreamPtr->mEncCtxPtr->time_base) <= 0)) {
            mEncodeVideo = writeVideoFrame(nullptr);
        }
        else mEncodeAudio = writeAudioFrame();
    }

    av_write_trailer(mFmtCtxPtr);

    // /* Close each codec. */
    if (mVideoStreamPtr) mVideoStreamPtr->free();
    if (mAudioStreamPtr) mAudioStreamPtr->free();

    if (!(mFmtCtxPtr->oformat->flags & AVFMT_NOFILE))
    //     /* Close the output file. */
        avio_closep(&mFmtCtxPtr->pb);

    /* free the stream */
    avformat_free_context(mFmtCtxPtr);
}

}