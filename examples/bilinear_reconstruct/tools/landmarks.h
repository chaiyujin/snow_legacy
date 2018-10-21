#pragma once
#include <snow.h>
#include <iostream>

class Landmarks {
    std::vector<snow::float2> mData;
    int64_t                   mTimestamp;
public:
    static const int Numbers = 73;
    Landmarks() : mData(0), mTimestamp(-1) {}
    int64_t timestamp() const { return mTimestamp; }

    snow::float2 &operator[](size_t i) { return mData[i]; }
    const snow::float2 &operator[](size_t i) const { return mData[i]; }
    const std::vector<snow::float2> &data() const { return mData; }
    friend std::istream &operator>>(std::istream &in, Landmarks &lm);
};

inline std::istream &operator>>(std::istream &in, Landmarks &lm) {
    int state, num;
    float x, y;
    if (in >> lm.mTimestamp) {
        in >> state >> num;
        lm.mData.resize(num);
        // skip pts, dde
        for (int i = 0; i < num; ++i) in >> x >> y;
        for (int i = 0; i < num; ++i) in >> x >> y;
        for (int i = 0; i < Landmarks::Numbers; ++i) {
            in >> x >> y;
            x = x * 2.f - 1.f;
            y = 1.f - y * 2.f;
            lm.mData[i] = { x, y };
        }
        for (int i = 0; i < num; ++i) in >> x >> y;
        std::string str;
        std::getline(in, str);
    }
    return in;
}
