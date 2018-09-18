#pragma once
#include <vector>
#include <snow.h>
#include "tensor.h"

#define USE_75_45

#ifndef USE_75_45
#define USE_50_25
#endif


class FaceDB
{
	FaceDB() {}
	~FaceDB() {}
	static std::vector<int>				    _tensor_shape;
	static std::vector<int>					_origin_shape;
	static Tensor3							_core_tensor;

	static std::vector<double>				_iden_singular;
	static std::vector<double>				_expr_singular;
	/* static face information */
	static std::vector<snow::int3>				_triangles;
	static std::vector<std::vector<int>>	_faces_of_point;
	static std::vector<int>					_face_vertices;
	static std::vector<snow::int3>				_face_triangles;
	/* static contour candidates */
	static std::vector<std::vector<int>>	_contour_candidates;
	/* norms */
	static std::vector<snow::float3>		_v_norms;
	static std::vector<snow::float3>		_f_norms;
	static const std::vector<int>			_landmarks_73;
	static const std::vector<int>			_expr_mask;
	static const std::vector<int>			_expr_valid_at_reference;
	static const std::vector<snow::int3>			_landmarks_triangles;
public:
	static double MAX_ALLOWED_WEIGHT_RANGE;
	static int  ndim_vert() { return _tensor_shape[0]; }
	static int  ndim_iden() { return _tensor_shape[1]; }
	static int  ndim_expr() { return _tensor_shape[2]; }
	static int  n_vertices() { return _tensor_shape[0] / 3; }
	static int  n_triangles() { return _triangles.size(); }
	static const Tensor3 &core_tensor() { return _core_tensor; }
	static const std::vector<std::vector<int>> contours() { return _contour_candidates; }
	static const std::vector<double> &iden_singular() { return _iden_singular; }
	static const std::vector<double> &expr_singular() { return _expr_singular; }

	static const std::vector<snow::int3> &landmarks_triangles() { return _landmarks_triangles; }
	static const std::vector<snow::int3> &triangles() { return _triangles; }
	static const std::vector<int>  &face_vertices() { return _face_vertices; }
	static const std::vector<snow::int3> &face_triangles() { return _face_triangles; }
	static const std::vector<int>  &landmarks_73() { return _landmarks_73; }
	static const std::vector<int>  &expr_mask() { return _expr_mask; }
	static const std::vector<int>  &expr_valid_at_reference() { return _expr_valid_at_reference; }

    static const std::vector<snow::float3> &v_normals() { return _v_norms; }

	static void read_raw_information(std::string dir);
	static void save_bin(std::string dir);
	static void load_bin(std::string dir);
	static void query_core(const double *iden, const double *expr, Tensor3 &result);
	static void update_gl_points(const Tensor3 &vertices, float *points, const int *mask=NULL);
	static void update_gl_normal(const Tensor3 &vertices, float *normal, bool is_points_masked=false);
	static snow::float3 face_norm(const Tensor3 &vertices, int ti);
	static snow::float3 vertex_norm(const Tensor3 &vertice, int id);
};
