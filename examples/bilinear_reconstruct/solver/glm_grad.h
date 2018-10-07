#pragma once
#include <snow.h>

namespace glm {
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