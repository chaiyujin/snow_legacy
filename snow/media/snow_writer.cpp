#include "snow_writer.h"
#include <string>
#include <stdexcept>

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples)
{
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


static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    printf("try to write 0x%X, 0x%X\n", fmt_ctx, pkt);

    return av_write_frame(fmt_ctx, pkt);
    /* Write the compressed frame to the media file. */
    // return av_interleaved_write_frame(fmt_ctx, pkt);
}


namespace snow {

MediaWriter::MediaWriter(std::string filename)
    : mFilename(filename)
    , mFmtCtxPtr(nullptr)
    , mAudioCodecPtr(nullptr)
    , mVideoCodecPtr(nullptr)
    , mAudioStreamPtr(nullptr)
    , mVideoStreamPtr(nullptr) {
    
    av_register_all();

    /* allocate the output media context */
    avformat_alloc_output_context2(&mFmtCtxPtr, NULL, NULL, filename.c_str());
    if (!mFmtCtxPtr) {
        av_log(nullptr, AV_LOG_WARNING, "Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&mFmtCtxPtr, NULL, "mpeg", filename.c_str());
    }
    if (!mFmtCtxPtr) { throw std::runtime_error("could not alloc output context"); }

}


OutputStream * MediaWriter::addStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codecId, int width, int height, int fps, int sampleRate) {
    OutputStream *ost = new OutputStream;

    *codec = avcodec_find_encoder(codecId);
    if (!(*codec)) {
        av_log(nullptr, AV_LOG_ERROR, "Could not find encoder for '%s'\n", avcodec_get_name(codecId));
        exit(1);
    }
    ost->mStreamPtr = avformat_new_stream(oc, NULL);
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

void MediaWriter::setAudioTrack(const std::vector<float> &audio, int sampleRate) {
    mAudio = audio;
    mSampleIndex = 0;
    mSampleRate = sampleRate;
    AVOutputFormat *fmtPtr = mFmtCtxPtr->oformat;
    if (fmtPtr->audio_codec != AV_CODEC_ID_NONE) {
        delete mAudioStreamPtr;
        mAudioStreamPtr = addStream(mFmtCtxPtr, &mAudioCodecPtr, fmtPtr->audio_codec, 0, 0, 0, sampleRate);
    }
}

void MediaWriter::setAudioTrack(const std::vector<int16_t> &audio, int sampleRate) {
    printf("add %d\n", audio.size());
    std::vector<float> float_audio(audio.size());
    for (size_t i = 0; i < float_audio.size(); ++i) {
        float_audio[i] = ((float)audio[i] / 32767.0);
    }
    setAudioTrack(float_audio, sampleRate);
}

void MediaWriter::openAudio() {
    AVFormatContext *oc = mFmtCtxPtr;
    AVCodec *codec      = mAudioCodecPtr;
    OutputStream *ost   = mAudioStreamPtr;

    AVCodecContext *c;
    int nb_samples;
    int ret;
    AVDictionary *opt = NULL;

    c = ost->mEncCtxPtr;

    /* open it */
    ret = avcodec_open2(c, codec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open audio codec: %s\n", av_err2str(ret));
        exit(1);
    }

    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;

    ost->mFramePtr     = alloc_audio_frame(c->sample_fmt, c->channel_layout,
                                           c->sample_rate, nb_samples);
}

void MediaWriter::write() {
    if (mAudioCodecPtr && mAudioStreamPtr->mFramePtr == nullptr) openAudio();

    if (!(mFmtCtxPtr->oformat->flags & AVFMT_NOFILE)) {
        int ret = avio_open(&mFmtCtxPtr->pb, mFilename.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open '%s': %s\n", mFilename.c_str(),
                    av_err2str(ret));
            return;
        }
    }


    /* Write the stream header, if any. */
    int ret = avformat_write_header(mFmtCtxPtr, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file: %s\n",
                av_err2str(ret));
        return ;
    }


    bool encode_audio = true;
    while (encode_audio) {
        encode_audio = write_audio_frame();
    }
}

bool MediaWriter::write_audio_frame() {

    AVCodecContext *c;
    AVPacket pkt = { 0 }; // data and size must be 0;
    AVFrame *frame = mAudioStreamPtr->mFramePtr;
    int ret;
    int got_packet;
    int dst_nb_samples;
    float *q = (float*)frame->data[0];

    av_init_packet(&pkt);
    c = mAudioStreamPtr->mEncCtxPtr;

    if (mSampleIndex + frame->nb_samples <= mAudio.size()) {
        for (int i = 0; i < frame->nb_samples; ++i) {
            *q++ = mAudio[mSampleIndex++];
        }
        frame->pts = mAudioStreamPtr->mNextPts;
        mAudioStreamPtr->mNextPts += frame->nb_samples;

        printf("current index %d\n", mSampleIndex);

        ret = avcodec_encode_audio2(c, &pkt, frame, &got_packet);
        if (ret < 0) {
            fprintf(stderr, "Error encoding audio frame: %s\n", av_err2str(ret));
            exit(1);
        }

        printf("encode\n");

        if (got_packet) {
            ret = write_frame(mFmtCtxPtr, &c->time_base, mAudioStreamPtr->mStreamPtr, &pkt);
            if (ret < 0) {
                fprintf(stderr, "Error while writing audio frame: %s\n", av_err2str(ret));
                exit(1);
            }
        }

        printf("write\n");

        return (frame || got_packet) ? true : false;
    }

    return false;
}


}