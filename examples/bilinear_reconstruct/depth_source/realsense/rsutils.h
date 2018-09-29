#pragma once

// stl
#include <map>
#include <fstream>
#include <vector>
// snow
#include <snow.h>
// self
#include "../data.h"
#include "librealsense.h"


/*********************************
 * self defined rsutils          *
 *********************************/
namespace librealsense {

inline void read_parameters(const char *filename,
                            rs2_intrinsics &color_intr,
                            rs2_intrinsics &depth_intr,
                            rs2_extrinsics &depth2color,
                            float &depth_scale) {
    std::ifstream fin(filename);
	auto read_intr = [&](rs2_intrinsics &intr) -> void {
		static int id = 0;
		std::string name;
		int model;
		fin >> name >> intr.width >> name >> intr.height
			>> name >> intr.ppx   >> name >> intr.ppy
			>> name >> intr.fx    >> name >> intr.fy
			>> name >> model;
		while (name != "coeffs:") fin >> name;
		intr.model = (rs2_distortion)model;
		for (int i = 0; i < 5; ++i) {
			fin >> intr.coeffs[i];
		}
#ifdef TEST_REALSENSE
		printf("Intrinsics %d\n", id++);
		printf("%d %d %f %f %f %f\n", intr.width, intr.height, intr.ppx, intr.ppy, intr.fx, intr.fy);
		printf("%s", rs2_distortion_to_string(intr.model));
		for (int i = 0; i < 5; ++i) {
			printf(" %f", intr.coeffs[i]);
		}
		printf("\n");
#endif
	};

	auto read_extr = [&](rs2_extrinsics &extr) -> void {
		static int id = 0;
		std::string name;
		fin >> name;
		for (int i = 0; i < 9; ++i) {
			fin >> extr.rotation[i];
		}
		fin >> name;
		for (int i = 0; i < 3; ++i) {
			fin >> extr.translation[i];
		}
#ifdef TEST_REALSENSE
		printf("Extrinsics %d\n", id++);
		for (int i = 0; i < 9; ++i) printf("%f ", extr.rotation[i]); printf("\n");
		for (int i = 0; i < 3; ++i) printf("%f ", extr.translation[i]); printf("\n");
#endif
	};

	read_intr(color_intr);
	read_intr(depth_intr);
	read_extr(depth2color);

	std::string name;
	fin >> name >> depth_scale;

	fin.close();
}

inline std::istream &operator >> (std::istream &in, rs2_intrinsics &intr) {
	std::string str; int id;
	in >> intr.width;	std::getline(in, str);
	in >> intr.height;	std::getline(in, str);
	in >> intr.ppx;		std::getline(in, str);
	in >> intr.ppy;		std::getline(in, str);
	in >> intr.fx;		std::getline(in, str);
	in >> intr.fy;		std::getline(in, str);
	in >> id;			std::getline(in, str);
	intr.model = static_cast<rs2_distortion>(id);
	for (int i = 0; i < 5; ++i) {
		in >> intr.coeffs[i];
	}
#ifdef TEST_REALSENSE
	if (intr.model != rs2_distortion::RS2_DISTORTION_NONE) {
		/* clear distortion, because images are undistorted already. */
		printf("[Warning]: clear distortion, because images are undistorted already.");
		intr.model = rs2_distortion::RS2_DISTORTION_NONE;
		for (int i = 0; i < 5; ++i) intr.coeffs[i] = 0;
	}
#endif
	std::getline(in, str);
	return in;
}

inline std::istream &operator>>(std::istream &in, rs2_extrinsics &extr) {
	std::string str; std::getline(in, str);
	for (int i = 0; i < 9; ++i) in >> extr.rotation[i];
	std::getline(in, str); std::getline(in, str);
	for (int i = 0; i < 3; ++i) in >> extr.translation[i];
	std::getline(in, str);
	return in;
}

inline std::ostream &operator<<(std::ostream &out, rs2_intrinsics &intr) {
	out << intr.width       << " width  "       << "\n"
		<< intr.height      << " height "       << "\n"
		<< intr.ppx         << " ppx    "       << "\n"
		<< intr.ppy         << " ppy    "       << "\n"
		<< intr.fx          << " fx     "       << "\n"
		<< intr.fy          << " fy     "       << "\n"
		<< (int)intr.model  << " distortion "   << " " << rs2_distortion_to_string(intr.model) << "\n";
	for (int i = 0; i < 5; ++i) { out << intr.coeffs[i] << " "; } out << "\n";
	return out;
}

inline std::ostream &operator<<(std::ostream &out, rs2_extrinsics &extr) {
	out << "rotation\n";
	for (int i = 0; i < 3; ++i) { for (int j = 0; j < 3; ++j) { out << extr.rotation[i * 3 + j] << " "; } out << std::endl; }
	out << "translation\n";
	for (int i = 0; i < 3; ++i) { out << extr.translation[i] << " "; }
	out << std::endl;
	return out;
}


class RealSenseSoftwareDevice {
    // parameters
    rs2_intrinsics          mColorIntrinsics;
    rs2_intrinsics          mDepthIntrinsics;
    rs2_extrinsics          mDepth2ColorExtrinsics;
    rs2_extrinsics          mColor2DepthExtrinsics;
	float					mDepthScale;
    // data pointers
    Image                   mColorImg;
    Image                   mDepthImg;
    Image                   mAlignedColorImg;
    Image                   mColorizedDepthImg;
    // point cloud
    PointCloud              mPointCloud;

public:
    static const int COLOR_BPP = 4;
    static const int DEPTH_BPP = 2;
    static const int FPS       = 30;

    RealSenseSoftwareDevice();
    RealSenseSoftwareDevice(std::string paramPath);
    ~RealSenseSoftwareDevice();

    void initFrom(std::string paramPath);
    void updateFramePair(const uint8_t *color, const uint8_t *depth);
    void updatePointCloud();
    glm::mat4 projectMat(const rs2_intrinsics &intr) const;
    glm::mat4 transformMat(const rs2_extrinsics &extr) const;
    glm::mat4 depthProjection() const { return projectMat(mDepthIntrinsics); }
    glm::mat4 colorProjection() const { return projectMat(mColorIntrinsics); }
    glm::mat4 depth2colorTransform() const { return transformMat(mDepth2ColorExtrinsics); }

    /* get */
	const rs2_intrinsics &colorIntrinsics()         const { return mColorIntrinsics;        }
	const rs2_intrinsics &depthIntrinsics()         const { return mDepthIntrinsics;        }
	const rs2_extrinsics &depthToColorExtrinsics()  const { return mDepth2ColorExtrinsics;  }
    const Image &         colorImg()                const { return mColorImg;               }
    const Image &         depthImg()                const { return mDepthImg;               }
    const Image &         alignedColorImg()         const { return mAlignedColorImg;        }
    const Image &         colorizedDepthImg()       const { return mColorizedDepthImg;      }
	PointCloud &          pointCloud()                    { return mPointCloud;             }
	const PointCloud &    pointCloud()              const { return mPointCloud;             }

};

}