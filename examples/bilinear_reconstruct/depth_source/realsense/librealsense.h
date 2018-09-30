/**
 * Copy from librealsense2
 * The code only use some basic types.
 * With copied codes, it's not needed to link librealsense any more.
 * */
#pragma once
#include <cmath>
#include <string>
#include <algorithm>
#include <assert.h>

namespace librealsense {
/**
 * Copy internal functions from librealsense2
 * */
typedef enum rs2_distortion
{
    RS2_DISTORTION_NONE                  , /**< Rectilinear images. No distortion compensation required. */
    RS2_DISTORTION_MODIFIED_BROWN_CONRADY, /**< Equivalent to Brown-Conrady distortion, except that tangential distortion is applied to radially distorted points */
    RS2_DISTORTION_INVERSE_BROWN_CONRADY , /**< Equivalent to Brown-Conrady distortion, except undistorts image instead of distorting it */
    RS2_DISTORTION_FTHETA                , /**< F-Theta fish-eye distortion model */
    RS2_DISTORTION_BROWN_CONRADY         , /**< Unmodified Brown-Conrady distortion model */
    RS2_DISTORTION_COUNT                   /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
} rs2_distortion;
const char* rs2_distortion_to_string(rs2_distortion distortion);

typedef enum rs2_format
{
    RS2_FORMAT_ANY             , /**< When passed to enable stream, librealsense will try to provide best suited format */
    RS2_FORMAT_Z16             , /**< 16-bit linear depth values. The depth is meters is equal to depth scale * pixel value. */
    RS2_FORMAT_DISPARITY16     , /**< 16-bit float-point disparity values. Depth->Disparity conversion : Disparity = Baseline*FocalLength/Depth. */
    RS2_FORMAT_XYZ32F          , /**< 32-bit floating point 3D coordinates. */
    RS2_FORMAT_YUYV            , /**< 32-bit y0, u, y1, v data for every two pixels. Similar to YUV422 but packed in a different order - https://en.wikipedia.org/wiki/YUV */
    RS2_FORMAT_RGB8            , /**< 8-bit red, green and blue channels */
    RS2_FORMAT_BGR8            , /**< 8-bit blue, green, and red channels -- suitable for OpenCV */
    RS2_FORMAT_RGBA8           , /**< 8-bit red, green and blue channels + constant alpha channel equal to FF */
    RS2_FORMAT_BGRA8           , /**< 8-bit blue, green, and red channels + constant alpha channel equal to FF */
    RS2_FORMAT_Y8              , /**< 8-bit per-pixel grayscale image */
    RS2_FORMAT_Y16             , /**< 16-bit per-pixel grayscale image */
    RS2_FORMAT_RAW10           , /**< Four 10-bit luminance values encoded into a 5-byte macropixel */
    RS2_FORMAT_RAW16           , /**< 16-bit raw image */
    RS2_FORMAT_RAW8            , /**< 8-bit raw image */
    RS2_FORMAT_UYVY            , /**< Similar to the standard YUYV pixel format, but packed in a different order */
    RS2_FORMAT_MOTION_RAW      , /**< Raw data from the motion sensor */
    RS2_FORMAT_MOTION_XYZ32F   , /**< Motion data packed as 3 32-bit float values, for X, Y, and Z axis */
    RS2_FORMAT_GPIO_RAW        , /**< Raw data from the external sensors hooked to one of the GPIO's */
    RS2_FORMAT_6DOF            , /**< Pose data packed as floats array, containing translation vector, rotation quaternion and prediction velocities and accelerations vectors */
    RS2_FORMAT_DISPARITY32     , /**< 32-bit float-point disparity values. Depth->Disparity conversion : Disparity = Baseline*FocalLength/Depth */
    RS2_FORMAT_COUNT             /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
} rs2_format;
const char* rs2_format_to_string(rs2_format format);

typedef struct rs2_intrinsics
{
    int           width;     /**< Width of the image in pixels */
    int           height;    /**< Height of the image in pixels */
    float         ppx;       /**< Horizontal coordinate of the principal point of the image, as a pixel offset from the left edge */
    float         ppy;       /**< Vertical coordinate of the principal point of the image, as a pixel offset from the top edge */
    float         fx;        /**< Focal length of the image plane, as a multiple of pixel width */
    float         fy;        /**< Focal length of the image plane, as a multiple of pixel height */
    rs2_distortion model;    /**< Distortion model of the image */
    float         coeffs[5]; /**< Distortion coefficients, order: k1, k2, p1, p2, k3 */
} rs2_intrinsics;

typedef struct rs2_extrinsics
{
    float rotation[9];    /**< Column-major 3x3 rotation matrix */
    float translation[3]; /**< Three-element translation vector, in meters */
} rs2_extrinsics;


	
/* Given a point in 3D space, compute the corresponding pixel coordinates in an image with no distortion or forward distortion coefficients produced by the same camera */
inline void rs2_project_point_to_pixel(float pixel[2], const struct rs2_intrinsics * intrin, const float point[3])
{
    //assert(intrin->model != RS2_DISTORTION_INVERSE_BROWN_CONRADY); // Cannot project to an inverse-distorted image

    float x = point[0] / point[2], y = point[1] / point[2];

    if(intrin->model == RS2_DISTORTION_MODIFIED_BROWN_CONRADY)
    {

        float r2  = x*x + y*y;
        float f = 1 + intrin->coeffs[0]*r2 + intrin->coeffs[1]*r2*r2 + intrin->coeffs[4]*r2*r2*r2;
        x *= f;
        y *= f;
        float dx = x + 2*intrin->coeffs[2]*x*y + intrin->coeffs[3]*(r2 + 2*x*x);
        float dy = y + 2*intrin->coeffs[3]*x*y + intrin->coeffs[2]*(r2 + 2*y*y);
        x = dx;
        y = dy;
    }
    if (intrin->model == RS2_DISTORTION_FTHETA)
    {
        float r = sqrtf(x*x + y*y);
        float rd = (float)(1.0f / intrin->coeffs[0] * atan(2 * r* tan(intrin->coeffs[0] / 2.0f)));
        x *= rd / r;
        y *= rd / r;
    }

    pixel[0] = x * intrin->fx + intrin->ppx;
    pixel[1] = y * intrin->fy + intrin->ppy;
}

/* Given pixel coordinates and depth in an image with no distortion or inverse distortion coefficients, compute the corresponding point in 3D space relative to the same camera */
inline void rs2_deproject_pixel_to_point(float point[3], const struct rs2_intrinsics * intrin, const float pixel[2], float depth)
{
    assert(intrin->model != RS2_DISTORTION_MODIFIED_BROWN_CONRADY); // Cannot deproject from a forward-distorted image
    assert(intrin->model != RS2_DISTORTION_FTHETA); // Cannot deproject to an ftheta image
    //assert(intrin->model != RS2_DISTORTION_BROWN_CONRADY); // Cannot deproject to an brown conrady model

    float x = (pixel[0] - intrin->ppx) / intrin->fx;
    float y = (pixel[1] - intrin->ppy) / intrin->fy;
    if(intrin->model == RS2_DISTORTION_INVERSE_BROWN_CONRADY)
    {
        float r2  = x*x + y*y;
        float f = 1 + intrin->coeffs[0]*r2 + intrin->coeffs[1]*r2*r2 + intrin->coeffs[4]*r2*r2*r2;
        float ux = x*f + 2*intrin->coeffs[2]*x*y + intrin->coeffs[3]*(r2 + 2*x*x);
        float uy = y*f + 2*intrin->coeffs[3]*x*y + intrin->coeffs[2]*(r2 + 2*y*y);
        x = ux;
        y = uy;
    }
    point[0] = depth * x;
    point[1] = depth * y;
    point[2] = depth;
}

/* Transform 3D coordinates relative to one sensor to 3D coordinates relative to another viewpoint */
inline void rs2_transform_point_to_point(float to_point[3], const struct rs2_extrinsics * extrin, const float from_point[3])
{
    to_point[0] = extrin->rotation[0] * from_point[0] + extrin->rotation[3] * from_point[1] + extrin->rotation[6] * from_point[2] + extrin->translation[0];
    to_point[1] = extrin->rotation[1] * from_point[0] + extrin->rotation[4] * from_point[1] + extrin->rotation[7] * from_point[2] + extrin->translation[1];
    to_point[2] = extrin->rotation[2] * from_point[0] + extrin->rotation[5] * from_point[1] + extrin->rotation[8] * from_point[2] + extrin->translation[2];
}
	
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
