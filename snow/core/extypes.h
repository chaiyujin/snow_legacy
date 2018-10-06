/**
 * The extended types for easy math
 * yuki-snow
 * */
#pragma once
#include <map>
#include <cmath>
#include <vector>
#include <stdexcept>

namespace snow {

#ifdef __DEBUG__
inline void __CheckRange(int i, int l, int r) { if (i < l || i >= r) throw std::runtime_error("out of range"); }
#else
inline void __CheckRange(int i, int l, int r) {}
#endif

template <typename T>
struct _int3 {
    T x, y, z;
    T *operator&()                     { return &x; }
    const T *operator&()         const { return &x; }
    T &operator[](int i)               { __CheckRange(i, 0, 3); return (&x)[i]; }
    const T &operator[](int i)   const { __CheckRange(i, 0, 3); return (&x)[i]; }

    const _int3<T>& operator+=(const _int3<T> &b) { x+=b.x; y+=b.y; z+=b.z; return *this; }
    const _int3<T>& operator-=(const _int3<T> &b) { x-=b.x; y-=b.y; z-=b.z; return *this; }
    const _int3<T>& operator*=(int t)             { x*=t;   y*=t;   z*=t;   return *this; }
    const _int3<T>& operator/=(int t)             { x/=t;   y/=t;   z/=t;   return *this; }
};
template <typename T> inline bool     operator <(const _int3<T> &a, const _int3<T> &b)      { return (a.x < b.x) || (a.x == b.x && a.y < b.y) || (a.x == b.x && a.y == b.y && a.z < b.z); }
template <typename T> inline bool     operator==(const _int3<T> &a, const _int3<T> &b)      { return a.x == b.x && a.y == b.y && a.z == b.z; }
template <typename T> inline bool     operator<=(const _int3<T> &a, const _int3<T> &b)      { return (a < b) || (a == b); }
template <typename T> inline _int3<T> operator +(const _int3<T> &a, const _int3<T> &b)      { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
template <typename T> inline _int3<T> operator -(const _int3<T> &a, const _int3<T> &b)      { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
template <typename T> inline _int3<T> operator *(const _int3<T> &a, int t)                  { return {a.x * t, a.y * t, a.z * t}; }
template <typename T> inline _int3<T> operator /(const _int3<T> &a, int t)                  { return {a.x / t, a.y / t, a.z / t}; }
template <typename T> inline float    euclidean_norm(const _int3<T> &a)                     { return std::sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }

template <typename T>
struct _int2 {
    T x, y;
    T *operator&()                    { return &x; }
    const T *operator&()        const { return &x; }
    T &operator[](int i)              { __CheckRange(i, 0, 2); return (&x)[i]; }
    const T &operator[](int i)  const { __CheckRange(i, 0, 2); return (&x)[i]; }

    const _int2<T>& operator+=(const _int2<T> &b) { x+=b.x; y+=b.y; return *this; }
    const _int2<T>& operator-=(const _int2<T> &b) { x-=b.x; y-=b.y; return *this; }
    const _int2<T>& operator*=(int t)             { x*=t;   y*=t;   return *this; }
    const _int2<T>& operator/=(int t)             { x/=t;   y/=t;   return *this; }
};
template <typename T> inline bool     operator <(const _int2<T> &a, const _int2<T> &b)      { return (a.x < b.x) || (a.x == b.x && a.y < b.y); }
template <typename T> inline bool     operator==(const _int2<T> &a, const _int2<T> &b)      { return a.x == b.x && a.y == b.y; }
template <typename T> inline bool     operator<=(const _int2<T> &a, const _int2<T> &b)      { return (a < b) || (a == b); }
template <typename T> inline _int2<T> operator +(const _int2<T> &a, const _int2<T> &b)      { return {a.x+b.x, a.y+b.y}; }
template <typename T> inline _int2<T> operator -(const _int2<T> &a, const _int2<T> &b)      { return {a.x-b.x, a.y-b.y}; }
template <typename T> inline _int2<T> operator *(const _int2<T> &a, int t)                  { return {a.x * t, a.y * t}; }
template <typename T> inline _int2<T> operator /(const _int2<T> &a, int t)                  { return {a.x / t, a.y / t}; }
template <typename T> inline float    euclidean_norm(const _int2<T> &a)                     { return std::sqrt(a.x*a.x + a.y*a.y); }

template <typename T>
struct _float3 {
    T x, y, z;
    T *operator&()                    { return &x; }
    const T *operator&()        const { return &x; }
    T &operator[](int i)              { __CheckRange(i, 0, 3); return (&x)[i]; }
    const T &operator[](int i)  const { __CheckRange(i, 0, 3); return (&x)[i]; }

    const _float3<T>& operator+=(const _float3<T> &b)       { x+=b.x;  y+=b.y;  z+=b.z;  return *this; }
    const _float3<T>& operator-=(const _float3<T> &b)       { x-=b.x;  y-=b.y;  z-=b.z;  return *this; }
    template <typename U> const _float3<T>& operator*=(U t) { x*=(T)t; y*=(T)t; z*=(T)t; return *this; }
    template <typename U> const _float3<T>& operator/=(U t) { x/=(T)t; y/=(T)t; z/=(T)t; return *this; }
};
template <typename T> inline bool       isnan(const _float3<T> &a)                              { return std::isnan(a.x) || std::isnan(a.y) || std::isnan(a.z); }
template <typename T> inline bool       operator <(const _float3<T> &a, const _float3<T> &b)    { return (a.x < b.x) || (a.x == b.x && a.y < b.y) || (a.x == b.x && a.y == b.y && a.z < b.z); }
template <typename T> inline bool       operator==(const _float3<T> &a, const _float3<T> &b)    { return a.x == b.x && a.y == b.y && a.z == b.z; }
template <typename T> inline bool       operator<=(const _float3<T> &a, const _float3<T> &b)    { return (a < b) || (a == b); }
template <typename T> inline _float3<T> operator +(const _float3<T>& a, const _float3<T>& b)    { return{ a.x + b.x, a.y + b.y, a.z + b.z }; }
template <typename T> inline _float3<T> operator -(const _float3<T>& a, const _float3<T>& b)    { return{ a.x - b.x, a.y - b.y, a.z - b.z }; }
template <typename T> inline _float3<T> operator *(const _float3<T>& a, T t)                    { return{ a.x * t,   a.y * t,   a.z * t }; }
template <typename T> inline _float3<T> operator /(const _float3<T>& a, T t)                    { return a * ((T)1.0 / t); }
template <typename T> inline _float3<T> cross(const _float3<T>& a,const _float3<T> &b)          { return { a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x }; }
template <typename T> inline T          dot(const _float3<T>& a,const _float3<T> &b)            { return a.x*b.x + a.y*b.y + a.z*b.z; }
template <typename T> inline T          euclidean_norm(const _float3<T> &a)                     { return std::sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }
template <typename T> inline _float3<T> normalize(const _float3<T> &a)                          { T t = euclidean_norm(a); return (t > 0) ? a * ((T)1.0 / t): a; }
template <typename T> inline bool       close(const _float3<T> &a, const _float3<T> &b, T eps=1e-5) { return std::abs(a.x-b.x)<eps && std::abs(a.y-b.y)<eps && std::abs(a.z-b.z)<eps; }
template <typename T> inline _float3<T> lerp(const _float3<T>& a, const _float3<T>& b, T t)     { return b * t + a * ((T)1.0 - t); }

template <typename T>
struct _float2 {
    T x, y;
    T *operator&()                    { return &x; }
    const T *operator&()        const { return &x; }
    T &operator[](int i)              { __CheckRange(i, 0, 2); return (&x)[i]; }
    const T &operator[](int i)  const { __CheckRange(i, 0, 2); return (&x)[i]; }

    const _float2<T>& operator+=(const _float3<T> &b)       { x+=b.x;  y+=b.y;  return *this; }
    const _float2<T>& operator-=(const _float3<T> &b)       { x-=b.x;  y-=b.y;  return *this; }
    template <typename U> const _float2<T>& operator*=(U t) { x*=(T)t; y*=(T)t; return *this; }
    template <typename U> const _float2<T>& operator/=(U t) { x/=(T)t; y/=(T)t; return *this; }
};
template <typename T> inline bool       isnan(const _float2<T> &a)                              { return std::isnan(a.x) || std::isnan(a.y); }
template <typename T> inline bool       operator <(const _float2<T> &a, const _float2<T> &b)    { return (a.x < b.x) || (a.x == b.x && a.y < b.y); }
template <typename T> inline bool       operator==(const _float2<T> &a, const _float2<T> &b)    { return a.x == b.x && a.y == b.y; }
template <typename T> inline bool       operator<=(const _float2<T> &a, const _float2<T> &b)    { return (a < b) || (a == b); }
template <typename T> inline _float2<T> operator +(const _float2<T> &a, const _float2<T> &b)    { return { a.x+b.x, a.y+b.y }; }
template <typename T> inline _float2<T> operator -(const _float2<T> &a, const _float2<T> &b)    { return { a.x-b.x, a.y-b.y }; }
template <typename T> inline _float2<T> operator *(const _float2<T> &a, T t)                    { return { a.x*t,   a.y*t };   }
template <typename T> inline _float2<T> operator /(const _float2<T> &a, T t)                    { return a * ((T)1.0 / t); }
template <typename T> inline T          dot(const _float2<T> &a, const _float2<T> &b)           { return a.x*b.x + a.y*b.y; }
template <typename T> inline T          cross(const _float2<T> &a, const _float2<T> &b)         { return a.x*b.y-a.y*b.x; }
template <typename T> inline T          euclidean_norm(const _float2<T> &a)                     { return std::sqrt(a.x*a.x + a.y*a.y); }
template <typename T> inline bool       close(const _float2<T> &a, const _float2<T> &b, T eps=1e-5) { return std::abs(a.x-b.x)<eps && std::abs(a.y-b.y)<eps; }

/* integer types */
typedef _int3<int32_t> int3;
typedef _int3<int64_t> long3;
typedef _int2<int32_t> int2;
typedef _int2<int64_t> long2;

/* float types */
typedef _float3<float>  float3;
typedef _float3<double> double3;
typedef _float2<float>  float2;
typedef _float2<double> double2;

/* === simple functions === */
template <typename T> T clamp(T x, const T &vmin, const T &vmax) { return (x < vmin) ? vmin : ( (x > vmax) ? vmax : x ); }

/* === color map === */

class color_map {
public:
    color_map() {}
    color_map(std::map<float, float3> map, int steps = 4000) : _map(map) { initialize(steps); }
    color_map(const std::vector<float3>& values, int steps = 4000) { for (size_t i = 0; i < values.size(); i++) { _map[(float)i / (values.size() - 1)] = values[i];} initialize(steps);}
    float min_key() const { return _min; }
    float max_key() const { return _max; }
    inline float3 get(float value) const { if (_max == _min) return *_data; auto t = (value - _min) / (_max - _min); t = clamp(t, 0.f, 1.f); return _data[(int)(t * (_size - 1))]; }

    static color_map jet() { return color_map({ { 0, 0, 255 }, { 0, 255, 255 }, { 255, 255, 0 }, { 255, 0, 0 }, { 50, 0, 0 } }); }

private:
    float3 calc(float value) const {
        if (_map.size() == 0) return{ value, value, value };
        // if we have exactly this value in the map, just return it
        if (_map.find(value) != _map.end()) return _map.at(value);
        // if we are beyond the limits, return the first/last element
        if (value < _map.begin()->first)   return _map.begin()->second;
        if (value > _map.rbegin()->first)  return _map.rbegin()->second;

        auto lower = _map.lower_bound(value) == _map.begin() ? _map.begin() : --(_map.lower_bound(value));
        auto upper = _map.upper_bound(value);

        auto t = (value - lower->first) / (upper->first - lower->first);
        auto c1 = lower->second;
        auto c2 = upper->second;
        return lerp(c1, c2, t);
    }

    void initialize(int steps) {
        if (_map.size() == 0) return;

        _min = _map.begin()->first;
        _max = _map.rbegin()->first;

        _cache.resize(steps + 1);
        for (int i = 0; i <= steps; i++) {
            auto t = (float)i / steps;
            auto x = _min + t*(_max - _min);
            _cache[i] = calc(x);
        }

        // Save size and data to avoid STL checks penalties in DEBUG
        _size = _cache.size();
        _data = _cache.data();
    }

    float                   _min, _max;
    std::map<float, float3> _map;
    std::vector<float3>     _cache;
    size_t _size; float3*   _data;
};

}
