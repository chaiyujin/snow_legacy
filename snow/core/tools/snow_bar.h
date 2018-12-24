#pragma once
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>
#ifdef __WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <stdio.h>
#endif

namespace snow {

inline int getConsoleWidth() {
#ifdef __WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return size.ws_col;
#endif
}

class ProgressBar {
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    struct iterator {
        int & operator* ()  { return curr ;   }

        iterator& operator++ () {
            ++curr;
            auto newTick = Clock::now();
            bar->mSilenceDura += toSec(bar->te, newTick);
            bar->te = newTick;
            double donePercent = (double)curr / (double)end;
            if (!bar->mIsMute &&
                (donePercent >= bar->mLastPercent + 0.01 ||
                 bar->mSilenceDura >= 1.0 ||
                 curr == end))
            {
                // update recording
                bar->mSilenceDura = 0.0;
                bar->mLastPercent = donePercent;
                // to console
                int totalWidth = getConsoleWidth();
                auto seconds = sec();
                std::string prev, post;
                /* get postfix */ {
                    std::stringstream ss;
                    ss << std::setw(3) << (int)(donePercent * 100) << "% ";
                    ss << std::setprecision(3) << std::fixed << seconds << "s ";
                    ss << std::setprecision(3) << std::fixed << curr / seconds << "it/s";
                    post = ss.str();
                }
                /* set prefix */ {
                    std::stringstream ss;
                    const int reserved = post.length() + 3;
                    int doneLength = donePercent * (totalWidth - reserved);
                    int leftLength = (totalWidth - reserved) - doneLength;
                    ss << "[";
                    for (int i = 0; i < doneLength; ++i) ss << "=";
                    for (int i = 0; i < leftLength; ++i) ss << " ";
                    ss << "] ";
                    prev = ss.str();
                }

                std::cout << prev << post;
                if (end == curr) std::cout << "\n"; else std::cout << "\r";
                std::cout.flush();
            }
            return *this;
        }

        bool operator== ( const iterator& that ) const { return curr == that.curr ; }
        bool operator!= ( const iterator& that ) const { return !(*this==that) ; }

        double sec() {
            return toSec(bar->ts, bar->te);
        }

        double toSec(const TimePoint &t0, TimePoint t1) {
            auto m = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
            return std::max((double)m / 1e6, 1e-6);
        }

        int curr;
        int end;
        ProgressBar *bar;
    };

    int mTotal;
    bool mIsMute;
    double mSilenceDura;
    double mLastPercent;
    TimePoint ts;
    TimePoint te;

public:
    ProgressBar(int total, bool mute=false)
        : mTotal(total)
        , mIsMute(mute)
        , mSilenceDura(0.0)
        , mLastPercent(0.0) {}

    iterator begin() {
        ts = Clock::now();
        return { 0, mTotal, this };
    }
    iterator end() {
        return { mTotal, mTotal, this };
    }
};

}