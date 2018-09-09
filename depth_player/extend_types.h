#pragma once
#include <cmath>
#include <stdexcept>

#ifdef __DEBUG__
inline void __CheckRange(int i, int l, int r) { if (i < l || i >= r) throw std::runtime_error("out of range"); }
#else
inline void __CheckRange(int i, int l, int r) {}
#endif

struct int3 {
	int x, y, z;
	int *operator&() 					 { return &x; }
	const int const*operator&()    const { return &x; }
	int &operator[](int i)               { __CheckRange(i, 0, 3); return (&x)[i]; }
	const int &operator[](int i)   const { __CheckRange(i, 0, 3); return (&x)[i]; }
};
inline bool operator <(const int3 &a, const int3 &b) { return (a.x < b.x) || (a.x == b.x && a.y < b.y) || (a.x == b.x && a.y == b.y && a.z < b.z); }
inline bool operator==(const int3 &a, const int3 &b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
inline bool operator<=(const int3 &a, const int3 &b) { return (a < b) || (a == b); }
inline float euclidean_norm(const int3 &a) 			 { return std::sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }

struct int2 {
	int x, y;
	int *operator&() 					{ return &x; }
	const int const*operator&() const 	{ return &x; }
	int &operator[](int i) 				{ __CheckRange(i, 0, 2); return (&x)[i]; }
	const int &operator[](int i) const  { __CheckRange(i, 0, 2); return (&x)[i]; }
};
inline bool operator <(const int2 &a, const int2 &b) { return (a.x < b.x) || (a.x == b.x && a.y < b.y); }
inline bool operator==(const int2 &a, const int2 &b) { return a.x == b.x && a.y == b.y; }
inline bool operator<=(const int2 &a, const int2 &b) { return (a < b) || (a == b); }
inline float euclidean_norm(const int2 &a) 			 { return std::sqrt(a.x*a.x + a.y*a.y); }

struct float3 {
	float x, y, z;
	float *operator&() 					  { return &x; }
	const float *operator&()  		const { return &x; }
	float &operator[](int i) 			  { __CheckRange(i, 0, 3); return (&x)[i]; }
	const float &operator[](int i)  const { __CheckRange(i, 0, 3); return (&x)[i]; }
	float dot(const float3 &b) 		const { return x * b.x + y * b.y + z * b.z; }
	float3 cross(const float3 &b) 	const { return { y*b.z-z*b.y, z*b.x-x*b.z, x*b.y-y*b.x }; }
	bool isnan() 					const { return std::isnan(x) || std::isnan(y) || std::isnan(z); }
};

inline bool   isnan(const float3 &a) { return std::isnan(a.x) || std::isnan(a.y) || std::isnan(a.z); }
inline float3 operator+(const float3& a, const float3& b) { return{ a.x + b.x, a.y + b.y, a.z + b.z }; }
inline float3 operator-(const float3& a, const float3& b) { return{ a.x - b.x, a.y - b.y, a.z - b.z }; }
inline float3 operator*(const float3& a, float t) { return{ a.x * t, a.y * t, a.z * t }; }
inline float3 operator/(const float3& a, float t) { return a * (1.0 / t); }
inline float  dot(const float3& a,const float3 &b) 		 { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline float3 cross(const float3& a,const float3 &b) 	 { return { a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x }; }

inline float  euclidean_norm(const float3 &a) 	  { return std::sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }
inline float3 normalize(const float3 &a)  		  { float t = euclidean_norm(a); return (t > 0) ? a * (1.0 / t): a; }
inline float3 lerp(const float3& a, const float3& b, float t) { return b * t + a * (1 - t); }

#define CLOSE_EPS 1e-4
struct float2
{
	float x, y;

	float length() const { return sqrt(x*x + y*y); }

	float *operator&()
	{
		return &x;
	}

	float2 normalize() const
	{
		return{ x / length(), y / length() };
	}

	bool operator==(const float2 &b) {
		return x == b.x && y == b.y;
	}

	bool close_to(const float2 &p)
	{
		if (abs(x - p.x) < CLOSE_EPS && abs(y - p.y) < CLOSE_EPS)
			return true;
		else
			return false;
	}

	float dot(const float2 &p)
	{
		return x * x + y * y;
	}

	float distance_to(const float2 &p)
	{
		return sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));
	}
};

inline float2 operator-(float2 a, float2 b)
{
	return{ a.x - b.x, a.y - b.y };
}

inline float2 operator*(float a, float2 b)
{
	return{ a * b.x, a * b.y };
}
