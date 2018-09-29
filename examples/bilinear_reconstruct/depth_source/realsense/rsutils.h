#pragma once

#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>
#include <librealsense2/hpp/rs_internal.hpp>
#include <librealsense2/hpp/rs_sensor.hpp>
#include <librealsense2/hpp/rs_processing.hpp>
// stl
#include <map>
#include <fstream>
#include <vector>
// snow
#include <snow.h>
#include "../data.h"

/**
 * Copy internal functions from librealsense2
 * */
namespace librealsense {
    /* align */
	typedef uint8_t byte;

	template<class GET_DEPTH, class TRANSFER_PIXEL>
	inline void align_images(const rs2_intrinsics& depth_intrin, const rs2_extrinsics& depth_to_other,
		                     const rs2_intrinsics& other_intrin, GET_DEPTH get_depth, TRANSFER_PIXEL transfer_pixel)
    {
		// Iterate over the pixels of the depth image
#pragma omp parallel for schedule(dynamic)
		for (int depth_y = 0; depth_y < depth_intrin.height; ++depth_y)
		{
			int depth_pixel_index = depth_y * depth_intrin.width;
			for (int depth_x = 0; depth_x < depth_intrin.width; ++depth_x, ++depth_pixel_index)
			{
				// Skip over depth pixels with the value of zero, we have no depth data so we will not write anything into our aligned images
				if (float depth = get_depth(depth_pixel_index))
				{
					// Map the top-left corner of the depth pixel onto the other image
					float depth_pixel[2] = { depth_x - 0.5f, depth_y - 0.5f }, depth_point[3], other_point[3], other_pixel[2];
					rs2_deproject_pixel_to_point(depth_point, &depth_intrin, depth_pixel, depth);
					rs2_transform_point_to_point(other_point, &depth_to_other, depth_point);
					rs2_project_point_to_pixel(other_pixel, &other_intrin, other_point);
					const int other_x0 = static_cast<int>(other_pixel[0] + 0.5f);
					const int other_y0 = static_cast<int>(other_pixel[1] + 0.5f);

					// Map the bottom-right corner of the depth pixel onto the other image
					depth_pixel[0] = depth_x + 0.5f; depth_pixel[1] = depth_y + 0.5f;
					rs2_deproject_pixel_to_point(depth_point, &depth_intrin, depth_pixel, depth);
					rs2_transform_point_to_point(other_point, &depth_to_other, depth_point);
					rs2_project_point_to_pixel(other_pixel, &other_intrin, other_point);
					const int other_x1 = static_cast<int>(other_pixel[0] + 0.5f);
					const int other_y1 = static_cast<int>(other_pixel[1] + 0.5f);

					if (other_x0 < 0 || other_y0 < 0 || other_x1 >= other_intrin.width || other_y1 >= other_intrin.height)
						continue;

					// Transfer between the depth pixels and the pixels inside the rectangle on the other image
					for (int y = other_y0; y <= other_y1; ++y)
					{
						for (int x = other_x0; x <= other_x1; ++x)
						{
							transfer_pixel(depth_pixel_index, y * other_intrin.width + x);
						}
					}
				}
			}
		}
	}

	inline void align_z_to_other(byte* z_aligned_to_other, const uint16_t* z_pixels, float z_scale, const rs2_intrinsics& z_intrin, const rs2_extrinsics& z_to_other, const rs2_intrinsics& other_intrin)
	{
		auto out_z = (uint16_t *)(z_aligned_to_other);
		align_images(z_intrin, z_to_other, other_intrin,
			[z_pixels, z_scale](int z_pixel_index)
		{
			return z_scale * z_pixels[z_pixel_index];
		},
			[out_z, z_pixels](int z_pixel_index, int other_pixel_index)
		{
			out_z[other_pixel_index] = out_z[other_pixel_index] ?
				std::min((int)out_z[other_pixel_index], (int)z_pixels[z_pixel_index]) :
				z_pixels[z_pixel_index];
		});
	}

	template<int N> struct bytes { char b[N]; };

	template<int N, class GET_DEPTH>
	inline void align_other_to_depth_bytes(byte* other_aligned_to_depth, GET_DEPTH get_depth, const rs2_intrinsics& depth_intrin, const rs2_extrinsics& depth_to_other, const rs2_intrinsics& other_intrin, const byte* other_pixels)
	{
		auto in_other = (const bytes<N> *)(other_pixels);
		auto out_other = (bytes<N> *)(other_aligned_to_depth);
		align_images(depth_intrin, depth_to_other, other_intrin, get_depth,
			[out_other, in_other](int depth_pixel_index, int other_pixel_index) { out_other[depth_pixel_index] = in_other[other_pixel_index]; });
	}

	template<class GET_DEPTH>
	inline void align_other_to_depth(byte* other_aligned_to_depth, GET_DEPTH get_depth, const rs2_intrinsics& depth_intrin, const rs2_extrinsics & depth_to_other, const rs2_intrinsics& other_intrin, const byte* other_pixels, rs2_format other_format)
	{
		switch (other_format)
		{
		case RS2_FORMAT_Y8:
			align_other_to_depth_bytes<1>(other_aligned_to_depth, get_depth, depth_intrin, depth_to_other, other_intrin, other_pixels);
			break;
		case RS2_FORMAT_Y16:
		case RS2_FORMAT_Z16:
			align_other_to_depth_bytes<2>(other_aligned_to_depth, get_depth, depth_intrin, depth_to_other, other_intrin, other_pixels);
			break;
		case RS2_FORMAT_RGB8:
		case RS2_FORMAT_BGR8:
			align_other_to_depth_bytes<3>(other_aligned_to_depth, get_depth, depth_intrin, depth_to_other, other_intrin, other_pixels);
			break;
		case RS2_FORMAT_RGBA8:
		case RS2_FORMAT_BGRA8:
			align_other_to_depth_bytes<4>(other_aligned_to_depth, get_depth, depth_intrin, depth_to_other, other_intrin, other_pixels);
			break;
		default:
			assert(false); // NOTE: rs2_align_other_to_depth_bytes<2>(...) is not appropriate for RS2_FORMAT_YUYV/RS2_FORMAT_RAW10 images, no logic prevents U/V channels from being written to one another
		}
	}

	inline void align_other_to_z(byte* other_aligned_to_z, const uint16_t* z_pixels, float z_scale, const rs2_intrinsics& z_intrin, const rs2_extrinsics& z_to_other, const rs2_intrinsics& other_intrin, const byte* other_pixels, rs2_format other_format)
	{
		align_other_to_depth(other_aligned_to_z, [z_pixels, z_scale](int z_pixel_index) { return z_scale * z_pixels[z_pixel_index]; }, z_intrin, z_to_other, other_intrin, other_pixels, other_format);
	}
}

/*********************************
 * self defined rsutils          *
 *********************************/
namespace librealsense_ext {

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

inline std::istream &operator>>(std::istream &in, rs2::software_sensor &sensor) {
	std::string str;
	int id = -1;
	float val;
	do {
		in >> id;
		if (id >= 0) {
			in >> val;
			std::getline(in, str);
			rs2_option option = static_cast<rs2_option>(id);
			sensor.add_read_only_option(static_cast<rs2_option>(id), val);
		}
	} while (id >= 0);
	std::getline(in, str);
	return in;
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

inline std::ostream &operator<<(std::ostream &out, rs2::sensor& sensor) {
	for (int i = 0; i < static_cast<int>(RS2_OPTION_COUNT); i++) {
		rs2_option option_type = static_cast<rs2_option>(i);
		if (sensor.supports(option_type)) {
			out << i << " " << sensor.get_option(option_type) << " ";
			out << option_type << std::endl;
		}
	}
	out << "-1\n";
	return out;
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
    rs2::software_device    mDevice;
    rs2::software_sensor    mColorSensor;
    rs2::software_sensor    mDepthSensor;
    rs2::syncer             mSyncer;
    int                     mCount;     // for syncer
    rs2::stream_profile     mColorProfile;
    rs2::stream_profile     mDepthProfile;
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
    glm::mat4 depthProjection() const { return projectMat(mDepthIntrinsics); }
    glm::mat4 colorProjection() const { return projectMat(mColorIntrinsics); }

    /* get */
	rs2::software_device &device()		                  { return mDevice;                 }
	rs2::software_sensor &colorSensor()                   { return mColorSensor;            }
	rs2::software_sensor &depthSensor()                   { return mDepthSensor;            }
	rs2::stream_profile & colorProfile()                  { return mColorProfile;           }
	rs2::stream_profile & depthProfile()                  { return mDepthProfile;           }
	rs2::syncer &         syncer()                        { return mSyncer;                 }
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