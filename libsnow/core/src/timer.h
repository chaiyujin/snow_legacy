#pragma once
#include <chrono>
#include <string>
#include "log.h"

namespace snow {

/**
 * To record runing time
 **/
class Timer {
    using clock = std::chrono::high_resolution_clock;
    using point = std::chrono::time_point<clock>;
protected:
    point mStarTime;
public:
    Timer() { restart(); }
    void restart() { mStarTime = clock::now(); }
    size_t microseconds() const { return std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - mStarTime).count(); }
    double milliseconds() const { return (double)microseconds() / 1000.0;    }
    double seconds()      const { return (double)microseconds() / 1000000.0; }
};

/**
 * auto stop at deconstruction  
 **/
class TimerGuard : public Timer {
private:
    std::string mTag;
    double      mDuration;
    bool        mStop;
public:
    TimerGuard(std::string tag="timer"): mTag(tag), mDuration(0), mStop(false) {}
    ~TimerGuard() { stop(); }

    double stop() {
        if (mStop) return mDuration;
        mDuration = milliseconds();
        int hours = (int)std::floor(mDuration / (3600000.0));
        int mins  = (int)std::floor((mDuration - hours * 3600000.0) / 60000.0);
        int secs  = (int)std::floor((mDuration - hours * 3600000.0 - mins * 60000.0) / 1000.0);
        double ms = (mDuration - hours * 3600000.0 - mins * 60000.0 - secs * 1000.0);
        if (hours > 0)     log::info("<{0}>: {1:d}h {2:d}m {3:d}s {4:.3f}ms", mTag, hours, mins, secs, ms);
        else if (mins > 0) log::info("<{0}>: {1:d}m {2:d}s {3:.3f}ms", mTag, mins, secs, ms);
        else if (secs > 0) log::info("<{0}>: {1:d}s {2:.3f}ms", mTag, secs, ms);
        else               log::info("<{0}>: {1:.3f}ms", mTag, ms);
        mStop = true;
        return mDuration;
    }
};

}