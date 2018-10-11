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
        snow::info("<{0}>: {1:.3f} ms", mTag, mDuration);
        mStop = true;
        return mDuration;
    }

private:
    std::string mTag;
    double      mDuration;
    bool        mStop;
};

}