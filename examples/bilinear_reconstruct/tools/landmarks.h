#pragma once
#include <snow.h>
#include <iostream>

class Landmarks {
    std::vector<snow::float2> mData;
    int64_t                   mTimestamp;
public:
    Landmarks() : mData(0), mTimestamp(0) {}

    const std::vector<snow::float2> &landmarks() const { return mData; }
    friend std::istream &operator>>(std::istream &in, Landmarks &lm);
};

inline std::istream &operator>>(std::istream &in, Landmarks &lm) {
    int state, num;
    float x, y;
    in >> lm.mTimestamp >> state >> num;
    lm.mData.resize(num);
    // skip pts, dde
    for (int i = 0; i < num; ++i) in >> x >> y;
    for (int i = 0; i < num; ++i) in >> x >> y;
    for (int i = 0; i < 73; ++i) {
        in >> x >> y;
        x = x * 2.f - 1.f;
        y = 1.f - y * 2.f;
        lm.mData[i] = { x, y };
    }
    for (int i = 0; i < num; ++i) in >> x >> y;
    return in;
}
