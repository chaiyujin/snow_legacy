#pragma once
#include <chrono>
#include <string>
#include <iostream>
#include "snow_log.h"

namespace snow {

class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    Timer() { restart(); }
    void    restart()        { mStarTime = Clock::now(); }
    // ms
    double  duration() const {
        auto m = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - mStarTime).count();
        return (double)m / 1000.0;
    }
protected:
    TimePoint mStarTime;
};

class StopWatch : public Timer {
public:
    StopWatch(std::string tag=""): mTag(tag), mDuration(0), mStop(false) {}
    ~StopWatch() { stop(); }

    double stop() {
        if (mStop) return mDuration;
        mDuration = duration();
        int hours = (int)std::floor(mDuration / (3600000.0));
        int mins  = (int)std::floor((mDuration - hours * 3600000.0) / 60000.0);
        int secs  = (int)std::floor((mDuration - hours * 3600000.0 - mins * 60000.0) / 1000.0);
        double ms = (mDuration - hours * 3600000.0 - mins * 60000.0 - secs * 1000.0);
        if (hours > 0)     snow::info("<{0}>: {1:2d}h {2:2d}m {3:2d}s {4:.3f}ms", mTag, hours, mins, secs, ms);
        else if (mins > 0) snow::info("<{0}>: {1:2d}m {2:2d}s {3:.3f}ms", mTag, mins, secs, ms);
        else if (secs > 0) snow::info("<{0}>: {1:2d}s {2:.3f}ms", mTag, secs, ms);
        else               snow::info("<{0}>: {1:.3f}ms", mTag, ms);
        mStop = true;
        return mDuration;
    }

private:
    std::string mTag;
    double      mDuration;
    bool        mStop;
};

}