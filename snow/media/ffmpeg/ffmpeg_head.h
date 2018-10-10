/**
 * The includer for ffmpeg components
 * Some functions are adapted to 
 * */
#pragma once

extern "C" {

#include <libavutil/opt.h>
#include <libavutil/dict.h>
#include <libavutil/eval.h>
#include <libavutil/fifo.h>
#include <libavutil/time.h>
#include <libavutil/avutil.h>
#include <libavutil/pixfmt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/rational.h>
#include <libavutil/timestamp.h>
#include <libavutil/samplefmt.h>
#include <libavutil/hwcontext.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/threadmessage.h>
#include <libavutil/channel_layout.h>
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

}

// replace av log macro
#undef av_ts2str
#undef av_err2str
#undef av_ts2timestr
#undef AV_TIME_BASE_Q
#define AV_TIME_BASE_Q AVRational {1, AV_TIME_BASE}

inline char *av_ts2str(int64_t ts) {
    static char str[AV_TS_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_ts_make_string(str, ts);
}

inline char* av_err2str(int errnum) {
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

inline char *av_ts2timestr(int64_t ts, AVRational tb) {
    static char str[AV_TS_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_ts_make_time_string(str, ts, &tb);
}
