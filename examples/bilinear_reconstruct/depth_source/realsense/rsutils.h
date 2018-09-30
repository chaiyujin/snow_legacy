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

class RealSenseSource {
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

    RealSenseSource();
    RealSenseSource(std::string paramPath);
    ~RealSenseSource();

    void initFrom(std::string paramPath);
    void updateFramePair(const uint8_t *color, const uint8_t *depth);
    void updatePointCloud();
    /* mat */
    glm::mat4 projectMat(const rs2_intrinsics &)    const;
    glm::mat4 transformMat(const rs2_extrinsics &)  const;
    glm::mat4 depthProjectionMat()                  const { return projectMat(mDepthIntrinsics); }
    glm::mat4 colorProjectionMat()                  const { return projectMat(mColorIntrinsics); }
    glm::mat4 depth2colorTransform()                const { return transformMat(mDepth2ColorExtrinsics); }
    glm::mat4 viewMat()                             const { glm::mat4 view(1.0f); view[1][1] = view[2][2] = -1.0f; return view; }
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