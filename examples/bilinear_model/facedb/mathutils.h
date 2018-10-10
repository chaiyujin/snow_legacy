#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"

#include "Eigen/Eigen"

typedef Eigen::Map<const Eigen::Matrix<double, -1, -1, Eigen::RowMajor>> MatMap;

/* std */
template <typename T>
T cumprod(const std::vector<T> &v)
{
	T ret(1);
	for (auto &i : v) ret *= i;
	return ret;
}

template <typename T> T deg2rad(T val) { return val / 180.0 * M_PI; }
template <typename T> T rad2deg(T val) { return val / M_PI * 180.0; }
template <typename T> T clamp_val(T x, T l, T r)
{
	if (x < l) return l;
	if (x > r) return r;
	return x;
}

namespace glm {
	inline void homo_coord(glm::dvec4 &q)
	{
		q.x /= q.w;
		q.y /= q.w;
		q.z /= q.w;
		q.w = 1.0;
	}

	template<typename T>
	GLM_FUNC_QUALIFIER tmat4x4<T, defaultp> dEulerAngleX
	(
		T const &angleX
	) {
		T cosX = glm::cos(angleX);
		T sinX = glm::sin(angleX);

		return tmat4x4<T, defaultp>(
			T(0), T(0), T(0), T(0),
			T(0), -sinX, cosX, T(0),
			T(0), -cosX, -sinX, T(0),
			T(0), T(0), T(0), T(0));
	}

	template<typename T>
	GLM_FUNC_QUALIFIER tmat4x4<T, defaultp> dEulerAngleY
	(
		T const &angleY
	) {
		T cosY = glm::cos(angleY);
		T sinY = glm::sin(angleY);

		return tmat4x4<T, defaultp>(
			-sinY, T(0), -cosY, T(0),
			T(0), T(0), T(0), T(0),
			cosY, T(0), -sinY, T(0),
			T(0), T(0), T(0), T(0));
	}

	template<typename T>
	GLM_FUNC_QUALIFIER tmat4x4<T, defaultp> dEulerAngleZ
	(
		T const &angleZ
	) {
		T cosZ = glm::cos(angleZ);
		T sinZ = glm::sin(angleZ);

		return tmat4x4<T, defaultp>(
			-sinZ, cosZ, T(0), T(0),
			-cosZ, -sinZ, T(0), T(0),
			T(0), T(0), T(0), T(0),
			T(0), T(0), T(0), T(0));
	}
}


/* --- self define types --- */

struct int3
{
	int x, y, z;
	int *operator&() { return &x; }
	const int const*operator&() const { return &x; }
	int &operator[](int i) {
		if (i < 0 || i > 2) throw std::runtime_error("int3 out of range.");
		return (&x)[i];
	}
	const int &operator[](int i) const {
		if (i < 0 || i > 2) throw std::runtime_error("int3 out of range.");
		return (&x)[i];
	}
	bool operator<(const int3 &b) const
	{
		return
			x < b.x ||
			(x == b.x && y < b.y) ||
			(x == b.x && y == b.y && z < b.z);
	}
};

struct int2
{
	int x, y;
	int *operator&() { return &x; }
	const int const*operator&() const { return &x; }
	int &operator[](int i) {
		if (i < 0 || i > 1) throw std::runtime_error("int2 out of range.");
		return (&x)[i];
	}
	const int &operator[](int i) const {
		if (i < 0 || i > 1) throw std::runtime_error("int2 out of range.");
		return (&x)[i];
	}
};

struct float3
{
	float x, y, z;

	float length() const { return sqrt(x*x + y*y + z*z); }

	float *operator&() {
		return &x;
	}
	const float *operator&() const {
		return &x;
	}

	float3 normalize() const
	{
		return (length() > 0) ? float3{ x / length(), y / length(), z / length() } : *this;
	}

	float dot(const float3 &b) const
	{
		return x * b.x + y * b.y + z * b.z;
	}

	float3 cross(const float3 &b) const
	{
		return{
			y * b.z - z * b.y,
			z * b.x - x * b.z,
			x * b.y - y * b.x
		};
	}

	bool isnan() const
	{
		return std::isnan(x) || std::isnan(y) || std::isnan(z);
	}

	glm::dvec4 to_dvec4() const { return glm::dvec4(x, y, z, 1); }
	glm::dvec3 to_dvec3() const { return glm::dvec3(x, y, z); }
	glm::dvec2 to_dvec2() const { return glm::dvec2(x, y); }
};

inline float3 operator*(const float3& a, float t)
{
	return{ a.x * t, a.y * t, a.z * t };
}

inline float3 operator/(const float3& a, float t)
{
	return{ a.x / t, a.y / t, a.z / t };
}

inline float3 operator+(const float3& a, const float3& b)
{
	return{ a.x + b.x, a.y + b.y, a.z + b.z };
}

inline float3 operator-(const float3& a, const float3& b)
{
	return{ a.x - b.x, a.y - b.y, a.z - b.z };
}

inline float3 lerp(const float3& a, const float3& b, float t)
{
	return b * t + a * (1 - t);
}

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


class bbox
{
private:
	int2 _min;
	int2 _max;
public:
	bbox() : _min({ INT32_MAX, INT32_MAX }), _max({ INT32_MIN, INT32_MIN }) {}
	bbox(int2 min, int2 max) : _min(min), _max(max) {}

	void add_point(const int2 &p)
	{
		_min.x = std::min(_min.x, p.x);
		_min.y = std::min(_min.y, p.y);
		_max.x = std::max(_max.x, p.x);
		_max.y = std::max(_max.y, p.y);
	}

	int area() const
	{
		if (_max.x >= _min.x && _max.y >= _min.y)
			return (_max.x - _min.x) * (_max.y - _min.y);
		else
			return 0;
	}

	bbox overlap(const bbox &b) const
	{
		bbox ret;
		ret._min.x = std::max(_min.x, b._min.x);
		ret._min.y = std::max(_min.y, b._min.y);
		ret._max.x = std::min(_max.x, b._max.x);
		ret._max.y = std::min(_max.y, b._max.y);
		return ret;
	}
};

typedef std::pair<float2, float2> line2f;

/* color map */

class color_map
{
public:
	color_map(std::map<float, float3> map, int steps = 4000) : _map(map)
	{
		initialize(steps);
	}

	color_map(const std::vector<float3>& values, int steps = 4000)
	{
		for (size_t i = 0; i < values.size(); i++)
		{
			_map[(float)i / (values.size() - 1)] = values[i];
		}
		initialize(steps);
	}

	color_map() {}

	inline float3 get(float value) const
	{
		if (_max == _min) return *_data;
		auto t = (value - _min) / (_max - _min);
		t = clamp_val(t, 0.f, 1.f);
		return _data[(int)(t * (_size - 1))];
	}

	float min_key() const { return _min; }
	float max_key() const { return _max; }

private:
	inline float3 lerp(const float3& a, const float3& b, float t) const
	{
		return b * t + a * (1 - t);
	}

	float3 calc(float value) const
	{
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

	void initialize(int steps)
	{
		if (_map.size() == 0) return;

		_min = _map.begin()->first;
		_max = _map.rbegin()->first;

		_cache.resize(steps + 1);
		for (int i = 0; i <= steps; i++)
		{
			auto t = (float)i / steps;
			auto x = _min + t*(_max - _min);
			_cache[i] = calc(x);
		}

		// Save size and data to avoid STL checks penalties in DEBUG
		_size = _cache.size();
		_data = _cache.data();
	}

	std::map<float, float3> _map;
	std::vector<float3> _cache;
	float _min, _max;
	size_t _size; float3* _data;
};


template <typename T>
inline bool comp_func(const T &p0, const T &p1)
{
	float m0 = sqrt((float)(p0.x * p0.x + p0.y * p0.y));
	float m1 = sqrt((float)(p1.x * p1.x + p1.y * p1.y));
	float v0 = p0.x / m0;
	float v1 = p1.x / m1;
	return (v0 > v1 || (v0 == v1 && m0 < m1));
};

template <typename T>
inline bool equal_func(const T &p0, const T &p1)
{
	return p0.x == p1.x && p0.y == p1.y;
}

/* convex */
template <class T>
void GrahamScan(const std::vector<T> &points, std::vector<T> &convex_hull) {
	if (points.size() < 3) {
		return;
	}
	convex_hull.clear();
	size_t si = 0;
	T pt_base = points[0];
	for (size_t i = 1; i < points.size(); ++i) {
		if (points[i].y > pt_base.y || (points[i].y == pt_base.y && points[i].x > pt_base.x)) {
			pt_base = points[i];
			si = i;
		}
	}
	std::vector<T> angles;
	for (size_t i = 0; i < points.size(); ++i) {
		if (si == i) continue;
		angles.push_back(points[i] - pt_base);
	}

	// sort in angles
	std::sort(angles.begin(), angles.end(), &comp_func<T>);
	// delete same vectors
	auto it = std::unique(angles.begin(), angles.end(), &equal_func<T>);
	angles.erase(it, angles.end());
	for (int i = (int)angles.size() - 1; i > 0; --i) {
		int j = i - 1;
		angles[i].x -= angles[j].x;
		angles[i].y -= angles[j].y;
	}
	convex_hull.push_back(angles[0]);
	for (int i = 1; i < angles.size(); ++i) {
		while (convex_hull.size()) {
			float v0 = angles[i].x * convex_hull.back().y;
			float v1 = angles[i].y * convex_hull.back().x;
			// cross < 0 || cross == 0 && same direction
			if (v0 > v1 || (v0 == v1 && angles[i].x * convex_hull.back().x > 0 && angles[i].y * convex_hull.back().y > 0)) {
				break;
			}
			else {
				angles[i].x += convex_hull.back().x;
				angles[i].y += convex_hull.back().y;
				convex_hull.pop_back();
			}
		}
		convex_hull.push_back(angles[i]);
	}
	convex_hull.front().x += pt_base.x;
	convex_hull.front().y += pt_base.y;
	for (int i = 1; i < convex_hull.size(); ++i) {
		convex_hull[i].x += convex_hull[i - 1].x;
		convex_hull[i].y += convex_hull[i - 1].y;
	}
	convex_hull.push_back(pt_base);
}
