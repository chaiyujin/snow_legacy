#pragma once

#include <snow.h>
#include <vector>

template <class T>
std::vector<snow::_float2<T>> getContourGrahamScan(const std::vector<snow::_float2<T>> &points) {
	if (points.size() < 3) {
        throw std::runtime_error("cannot calculate contour for points less than 3.");
	}
    std::vector<snow::_float2<T>> convexHull;
	size_t si = 0;
	snow::_float2<T> ptBase = points[0];
	for (size_t i = 1; i < points.size(); ++i) {
		if (points[i].y > ptBase.y || (points[i].y == ptBase.y && points[i].x > ptBase.x)) {
			ptBase = points[i];
			si = i;
		}
	}
	std::vector<snow::_float2<T>> angles;
	for (size_t i = 0; i < points.size(); ++i) {
		if (si == i) continue;
		angles.push_back(points[i] - ptBase);
	}
    auto compFunc = [](const snow::_float2<T> &p0, const snow::_float2<T> &p1) -> bool {
        float m0 = sqrt((float)(p0.x * p0.x + p0.y * p0.y));
        float m1 = sqrt((float)(p1.x * p1.x + p1.y * p1.y));
        float v0 = p0.x / m0;
        float v1 = p1.x / m1;
        return (v0 > v1 || (v0 == v1 && m0 < m1));
    };
	// sort in angles
	std::sort(angles.begin(), angles.end(), compFunc);
	// delete same vectors
	auto it = std::unique(angles.begin(), angles.end());
	angles.erase(it, angles.end());
	for (int i = (int)angles.size() - 1; i > 0; --i) {
		int j = i - 1;
		angles[i].x -= angles[j].x;
		angles[i].y -= angles[j].y;
	}
	convexHull.push_back(angles[0]);
	for (int i = 1; i < angles.size(); ++i) {
		while (convexHull.size()) {
			float v0 = angles[i].x * convexHull.back().y;
			float v1 = angles[i].y * convexHull.back().x;
			// cross < 0 || cross == 0 && same direction
			if (v0 > v1 || (v0 == v1 && angles[i].x * convexHull.back().x > 0 && angles[i].y * convexHull.back().y > 0)) {
				break;
			}
			else {
				angles[i].x += convexHull.back().x;
				angles[i].y += convexHull.back().y;
				convexHull.pop_back();
			}
		}
		convexHull.push_back(angles[i]);
	}
	convexHull.front().x += ptBase.x;
	convexHull.front().y += ptBase.y;
	for (int i = 1; i < convexHull.size(); ++i) {
		convexHull[i].x += convexHull[i - 1].x;
		convexHull[i].y += convexHull[i - 1].y;
	}
	convexHull.push_back(ptBase);
    return convexHull;
}
