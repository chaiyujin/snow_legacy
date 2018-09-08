#pragma once
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
