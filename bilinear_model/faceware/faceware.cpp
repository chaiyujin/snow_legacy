#include "faceware.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

double							FaceDB::MAX_ALLOWED_WEIGHT_RANGE = 1.25;
std::vector<int>				FaceDB::_tensor_shape;
std::vector<int>				FaceDB::_origin_shape;
Tensor3							FaceDB::_core_tensor;
std::vector<double>				FaceDB::_iden_singular;
std::vector<double>				FaceDB::_expr_singular;
/* static face information */
std::vector<snow::int3>				FaceDB::_triangles;
std::vector<std::vector<int>>	FaceDB::_faces_of_point;
std::vector<int>				FaceDB::_face_vertices;
std::vector<snow::int3>				FaceDB::_face_triangles;
/* static contour candidates */
std::vector<std::vector<int>>	FaceDB::_contour_candidates;
/* norms */
std::vector<snow::float3>				FaceDB::_v_norms;
std::vector<snow::float3>				FaceDB::_f_norms;

const std::vector<int> FaceDB::_landmarks_73({
	2580, 9759, 2573, 2748, 6740, 6770, 6541, 3611, 10573, 3865, 3854, 3842, 10652, 5500, 11284, 10751, 4247, 712, 709, 4245, 4248, 9251, 2136, 7168, 2134, 2123, 2124, 1965, 2171, 7100, 9445, 576, 4349, 10820, 4392, 9236, 6335, 1789, 6388, 6274, 10459, 10492, 283, 10733, 8994, 10453, 6150, 1620, 6090, 8814, 182, 3237, 174, 10306, 3231, 3190, 6117, 8864, 6164, 3272, 3275, 10334, 3280, 6144, 8940, 9310, 9447, 9443, 2170, 4339, 10880, 4356, 10687 
});

#ifndef USE_75_25
const std::vector<int> FaceDB::_expr_mask({
	0 , 1 , 1 , 0 , 0 ,
	0 , 0 , 0 , 0 , 1 ,
	1 , 0 , 0 , 0 , 0 ,
	1 , 1 , 1 , 1 , 1 ,
	0 , 1 , 1 , 1 , 1 ,
	1 , 1 , 1 , 1 , 1 ,
	1 , 1 , 1 , 1 , 1 ,
	1 , 1 , 1 , 0 , 1 ,
	1 , 1 , 1 , 1 , 1 ,
	0 , 0
});
#else
const std::vector<int> FaceDB::_expr_mask({
	0, 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1
});
#endif

#ifdef USE_75_25
const std::vector<int> FaceDB::_expr_valid_at_reference({
	1, 1, 1, 1, 1,
	0, 0, 1, 1, 1,
	1, 0, 0, 1, 1,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0
});
#else
const std::vector<int> FaceDB::_expr_valid_at_reference({
	1, 1, 1, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0
});
#endif

const std::vector<snow::int3> FaceDB::_landmarks_triangles({
	{46, 4, 5},
	{6, 57, 5},
	{57, 6, 56},
	{46, 5, 57},
	{2, 3, 46},
	{37, 2, 38},
	{63, 47, 46},
	{38, 46, 47},
	{38, 2, 46},
	{3, 4, 46},
	{1, 2, 37},
	{46, 57, 58},
	{48, 47, 63},
	{56, 59, 58},
	{55, 59, 56},
	{58, 57, 56},
	{48, 63, 62},
	{38, 44, 37},
	{44, 38, 39},
	{39, 64, 44},
	{49, 39, 48},
	{45, 64, 39},
	{39, 38, 48},
	{47, 48, 38},
	{37, 44, 36},
	{56, 6, 7},
	{64, 36, 44},
	{1, 27, 0},
	{27, 1, 66},
	{65, 66, 28},
	{66, 65, 27},
	{37, 66, 1},
	{27, 65, 21},
	{0, 27, 21},
	{23, 22, 26},
	{26, 22, 21},
	{26, 21, 65},
	{25, 26, 28},
	{28, 26, 65},
	{66, 30, 28},
	{28, 67, 68},
	{30, 37, 67},
	{29, 36, 35},
	{36, 29, 67},
	{66, 37, 30},
	{68, 67, 29},
	{23, 25, 24},
	{23, 26, 25},
	{29, 24, 68},
	{24, 29, 35},
	{18, 24, 35},
	{68, 24, 25},
	{28, 68, 25},
	{23, 24, 18},
	{35, 36, 42},
	{30, 67, 28},
	{67, 37, 36},
	{42, 36, 64},
	{54, 55, 8},
	{7, 55, 56},
	{60, 59, 54},
	{53, 60, 54},
	{8, 55, 7},
	{49, 62, 50},
	{49, 48, 62},
	{39, 40, 45},
	{61, 50, 62},
	{51, 50, 61},
	{39, 50, 40},
	{45, 42, 64},
	{55, 54, 59},
	{54, 9, 53},
	{9, 54, 8},
	{9, 10, 53},
	{53, 10, 52},
	{40, 51, 52},
	{40, 50, 51},
	{52, 11, 12},
	{11, 52, 10},
	{52, 12, 40},
	{61, 52, 51},
	{45, 40, 41},
	{60, 53, 52},
	{41, 40, 12},
	{42, 33, 43},
	{33, 42, 70},
	{41, 70, 42},
	{70, 41, 34},
	{42, 45, 41},
	{33, 70, 69},
	{43, 18, 35},
	{43, 33, 18},
	{19, 17, 18},
	{32, 19, 69},
	{17, 19, 20},
	{19, 18, 69},
	{33, 69, 18},
	{18, 17, 23},
	{69, 70, 32},
	{43, 35, 42},
	{34, 71, 72},
	{34, 41, 71},
	{71, 31, 72},
	{31, 71, 13},
	{31, 13, 14},
	{72, 31, 15},
	{41, 13, 71},
	{34, 72, 32},
	{20, 16, 17},
	{32, 72, 20},
	{20, 15, 16},
	{72, 15, 20},
	{31, 14, 15},
	{20, 19, 32},
	{34, 32, 70},
	{41, 12, 13},
	{49, 50, 39},
	{63, 58, 59},
	{59, 62, 63},
	{63, 46, 58},
	{60, 62, 59},
	{62, 60, 61},
	{60, 52, 61}
});

void FaceDB::read_raw_information(std::string dir)
{
	if (dir[dir.length() - 1] != '/' && dir[dir.length() - 1] != '\\')
		dir += '/';
	std::string tensor_path = dir + "tensor.bin";
	std::string face_path = dir + "triangles.txt";
	std::string cont_path = dir + "contourpoints.txt";
	std::string mask_path = dir + "face_mask.txt";
	std::string iden_s_path = dir + "iden_singular.txt";
	std::string expr_s_path = dir + "expr_singular.txt";

	if (!snow::exists({ tensor_path, face_path, cont_path, mask_path, iden_s_path, expr_s_path })) {
		printf("Failed to find tensor information file.\n");
        exit(1);
    }

    printf("[FaceDB]: begin to read\n");

	/* read tensor */
	auto tmp_alloc_read = [](float **tmp, FILE **fp, int size) -> void
	{
		delete[] *tmp;
		*tmp = new float[size];
		size_t num = fread(*tmp, sizeof(float), size, *fp);
	};

#ifdef USE_75_45
	{
		float *tmp = NULL;
		FILE *fp_tensor = fopen(tensor_path.c_str(), "rb");
		/* tensor size */
		_tensor_shape.resize(3);
		_origin_shape.resize(3);
        int read_size;
		read_size = fread(_tensor_shape.data(), sizeof(int), 3, fp_tensor);
		read_size = fread(_origin_shape.data(), sizeof(int), 3, fp_tensor);
		int size = _tensor_shape[0] * _tensor_shape[1] * _tensor_shape[2];
		tmp_alloc_read(&tmp, &fp_tensor, size);
		// unfold core tensor
		_core_tensor.resize(_tensor_shape);
		_core_tensor.unfoldData<float>(tmp, 1);
		// /* iden trans */
		// size = _tensor_shape[1] * _origin_shape[1];
		// tmp_alloc_read(&tmp, &fp_tensor, size);
		// _iden_UT.resize(_tensor_shape[1], _origin_shape[1]);
		// for (int i = 0; i < _tensor_shape[1]; ++i)
		// 	for (int j = 0; j < _origin_shape[1]; ++j)
		// 		_iden_UT(i, j) = tmp[i * _origin_shape[1] + j];
		// /* expr trans */
		// size = _tensor_shape[2] * _origin_shape[2];
		// tmp_alloc_read(&tmp, &fp_tensor, size);
		// _expr_UT.resize(_tensor_shape[2], _origin_shape[2]);
		// for (int i = 0; i < _tensor_shape[2]; ++i)
		// 	for (int j = 0; j < _origin_shape[2]; ++j)
		// 		_expr_UT(i, j) = tmp[i * _origin_shape[2] + j];
		/* close */
		delete[] tmp;
		fclose(fp_tensor);

		std::cout << " core  shape: " << _tensor_shape << std::endl;
		std::cout << "origin shape: " << _origin_shape << std::endl;
	}
#else
	{
		const std::string tensor_5025 = dir + "blendshape_core.tensor";
		const std::string prior_id = dir + "blendshape_u_0_aug.tensor";
		const std::string prior_ex = dir + "blendshape_u_1_aug.tensor";

		{
			std::fstream fin;
			fin.open(tensor_5025, std::ios::in | std::ios::binary);

			int l, m, n;
			fin.read(reinterpret_cast<char*>(&(l)), sizeof(int));
			fin.read(reinterpret_cast<char*>(&(m)), sizeof(int));
			fin.read(reinterpret_cast<char*>(&(n)), sizeof(int));

			std::cout << l << " " << m << " " << n << std::endl;
			double *tmp_tensor = new double[l * m * n];

			fin.read(reinterpret_cast<char*>(tmp_tensor), sizeof(double)*l*m*n);
			fin.close();

			_tensor_shape.resize(3);
			_origin_shape.resize(3);
			_tensor_shape[0] = n;  _origin_shape[0] = n;
			_tensor_shape[1] = l; _origin_shape[1] = 150;
			_tensor_shape[2] = m; _origin_shape[2] = 47;

			_core_tensor.resize({ n, l, m });
			_core_tensor.unfold_data(tmp_tensor, 2);

			std::cout << " core  shape: " << _tensor_shape << std::endl;
			std::cout << "origin shape: " << _origin_shape << std::endl;
			delete[] tmp_tensor;
		}

		{
			Eigen::MatrixXd sigma_Wid;
			std::fstream fin;
			fin.open(prior_id, std::ios::in | std::ios::binary);

			int ndims;
			fin.read(reinterpret_cast<char*>(&ndims), sizeof(int));
			std::cout << "identity prior dim = " << ndims << std::endl;

			_iden_avg.resize(ndims);
			_iden_0.resize(ndims);
			sigma_Wid.resize(ndims, ndims);

			fin.read(reinterpret_cast<char*>(_iden_avg.data()), sizeof(double)*ndims);
			fin.read(reinterpret_cast<char*>(_iden_0.data()), sizeof(double)*ndims);
			fin.read(reinterpret_cast<char*>(sigma_Wid.data()), sizeof(double)*ndims*ndims);
			_iden_inv_sigma = sigma_Wid.inverse();
			// Take the diagonal
			_iden_inv_sigma_diag = Eigen::VectorXd(ndims);
			for (int i = 0; i<ndims; ++i) {
				_iden_inv_sigma_diag(i) = _iden_inv_sigma(i, i);
			}
			/*std::cout << _iden_avg << std::endl;
			std::cout << _iden_inv_sigma_diag << std::endl;*/
			int m, n;
			fin.read(reinterpret_cast<char*>(&m), sizeof(int));
			fin.read(reinterpret_cast<char*>(&n), sizeof(int));
			std::cout << "iden_UT size: " << n << 'x' << m << std::endl;
			_iden_UT.resize(n, m);
			fin.read(reinterpret_cast<char*>(_iden_UT.data()), sizeof(double)*m*n);


			for (int i = 0; i < _iden_UT.rows(); ++i)
			{
				for (int j = 0; j < _iden_UT.cols(); ++j)
				{
					if (abs(_iden_UT(i, j)) < 1e-30)
						_iden_UT(i, j) = 0;
				}
			}


			_iden_max.resize(n);
			_iden_min.resize(n);
			for (int i = 0; i<n; ++i) {
				_iden_max(i) = _iden_avg(i) + (_iden_UT.row(i).maxCoeff() - _iden_avg(i)) * MAX_ALLOWED_WEIGHT_RANGE;
				_iden_min(i) = _iden_avg(i) + (_iden_UT.row(i).minCoeff() - _iden_avg(i)) * MAX_ALLOWED_WEIGHT_RANGE;
			}

			fin.close();
		}

		{
			Eigen::MatrixXd sigma_Wexp;
			std::fstream fin;
			fin.open(prior_ex, std::ios::in | std::ios::binary);

			int ndims;
			fin.read(reinterpret_cast<char*>(&ndims), sizeof(int));
			std::cout << "expr prior dim = " << ndims << std::endl;

			_expr_avg.resize(ndims);
			_expr_0.resize(ndims);
			sigma_Wexp.resize(ndims, ndims);

			fin.read(reinterpret_cast<char*>(_expr_avg.data()), sizeof(double)*ndims);
			fin.read(reinterpret_cast<char*>(_expr_0.data()), sizeof(double)*ndims);
			fin.read(reinterpret_cast<char*>(sigma_Wexp.data()), sizeof(double)*ndims*ndims);
			_expr_inv_sigma = sigma_Wexp.inverse();
			// Take the diagonal
			_expr_inv_sigma_diag = Eigen::VectorXd(ndims);
			for (int i = 0; i<ndims; ++i) {
				_expr_inv_sigma_diag(i) = _expr_inv_sigma(i, i);
			}
			/*std::cout << _expr_avg << std::endl;
			std::cout << _expr_inv_sigma_diag << std::endl;*/
			int m, n;
			fin.read(reinterpret_cast<char*>(&m), sizeof(int));
			fin.read(reinterpret_cast<char*>(&n), sizeof(int));
			std::cout << "expr_UT size: " << n << 'x' << m << std::endl;
			_expr_UT.resize(n, m);
			fin.read(reinterpret_cast<char*>(_expr_UT.data()), sizeof(double)*m*n);

			/*for (int i = 0; i < _expr_UT.rows(); ++i)
			{
				for (int j = 0; j < _expr_UT.cols(); ++j)
				{
					if (abs(_expr_UT(i, j)) < 1e-100)
						_expr_UT(i, j) = 0;
					else if (_expr_UT(i, j) > 1e100)
						_expr_UT(i, j) = 1e100;
					else if (_expr_UT(i, j) < -1e100)
						_expr_UT(i, j) = -1e100;
					else if (std::isnan(_expr_UT(i, j)))
						_expr_UT(i, j) = 0;
				}
			}*/

			_expr_max.resize(n);
			_expr_min.resize(n);
			for (int i = 0; i<n; ++i) {
				_expr_max(i) = _expr_avg(i) + (_expr_UT.row(i).maxCoeff() - _expr_avg(i)) * MAX_ALLOWED_WEIGHT_RANGE;
				_expr_min(i) = _expr_avg(i) + (_expr_UT.row(i).minCoeff() - _expr_avg(i)) * MAX_ALLOWED_WEIGHT_RANGE;
			}


			fin.close();
		}
	}
#endif


	/* read faces */
	{
		int tmp_vi[4], tmp_uvi[4], tmp_ni[4];
		FILE *fp_faces = fopen(face_path.c_str(), "r");
		_faces_of_point.clear();
		_triangles.clear();
		_faces_of_point.resize(_tensor_shape[0] / 3);
		int read_size;
        while (!feof(fp_faces))
		{
            while (fgetc(fp_faces) != 'f') {
                if (feof(fp_faces)) break;
            }
            if (feof(fp_faces)) break;
			read_size = fscanf(fp_faces, "%d/%d/%d", &tmp_vi[0], &tmp_uvi[0], &tmp_ni[0]);
			read_size = fscanf(fp_faces, "%d/%d/%d", &tmp_vi[1], &tmp_uvi[1], &tmp_ni[1]);
			read_size = fscanf(fp_faces, "%d/%d/%d", &tmp_vi[2], &tmp_uvi[2], &tmp_ni[2]);
			read_size = fscanf(fp_faces, "%d/%d/%d", &tmp_vi[3], &tmp_uvi[3], &tmp_ni[3]);
            
			--tmp_vi[0]; --tmp_vi[1]; --tmp_vi[2]; --tmp_vi[3];  // -1
			_triangles.push_back({ tmp_vi[0], tmp_vi[1], tmp_vi[2] });
			_faces_of_point[tmp_vi[0]].push_back(_triangles.size() - 1);
			_faces_of_point[tmp_vi[1]].push_back(_triangles.size() - 1);
			_faces_of_point[tmp_vi[2]].push_back(_triangles.size() - 1);
			_triangles.push_back({ tmp_vi[0], tmp_vi[2], tmp_vi[3] });
			_faces_of_point[tmp_vi[0]].push_back(_triangles.size() - 1);
			_faces_of_point[tmp_vi[2]].push_back(_triangles.size() - 1);
			_faces_of_point[tmp_vi[3]].push_back(_triangles.size() - 1);
		}
		fclose(fp_faces);
        printf("read face done\n");
	}
	/* read contours */
	{

		auto ReadFileByLine = [](const std::string &filename)
		{
			std::ifstream fin(filename);
			std::vector<std::string> lines;
			while (fin) {
				std::string line;
				std::getline(fin, line);
				if (!line.empty())
					lines.push_back(line);
			}
			return lines;
		};

		std::vector<std::string> lines = ReadFileByLine(cont_path);
		_contour_candidates.resize(lines.size());

		std::transform(lines.begin(), lines.end(), _contour_candidates.begin(),
			[](const std::string &line) {
			std::vector<int> indices;
			std::string word = "";
			int i = 0;
			while (i < line.length()) {
				while (i < line.length() && line[i] == ' ') ++i;
				while (i < line.length() && line[i] != ' ') word += line[i++];
				if (word.length() > 0) {
					indices.push_back(std::stoi(word));
					word = "";
				}
			}
			return indices;
		});
        printf("read contour done\n");
	}
	/* read singular values */
	{
		{
			std::ifstream fin(iden_s_path);
			double x;
			_iden_singular.clear();
			while (fin >> x)
			{
				_iden_singular.push_back(x);
			}
			fin.close();
		}
		{
			std::ifstream fin(expr_s_path);
			double x;
			_expr_singular.clear();
			while (fin >> x)
			{
				_expr_singular.push_back(x);
			}
			fin.close();
		}
        printf("read expr singular done\n");
	}
	/* read face only mesh */
	{
		std::ifstream fin(mask_path);
		if (fin.is_open())
		{
			_face_vertices.clear();
			_face_triangles.clear();
			int n;
			fin >> n;
			for (int i = 0; i < n; ++i)
			{
				int id;
				fin >> id;
				_face_vertices.push_back(id);
			}
			fin >> n;
			for (int i = 0; i < n; ++i)
			{
				int x, y, z;
				fin >> x >> y >> z;
				_face_triangles.push_back({ x, y, z });
			}
			fin.close();
		}
        printf("read face only mesh done\n");
	}
}

void FaceDB::query_core(const double *iden, const double *expr, Tensor3 &result)
{
	_core_tensor.mulVec(iden, expr, result);
}

void FaceDB::update_gl_points(const Tensor3 &vertices, float *points, const int *mask)
{
	for (size_t i = 0, idx = 0; i < _triangles.size(); ++i)
	{
		auto v3 = _triangles[i];
		if (mask) for (int j = 0; j < 3; ++j) if (0 == mask[v3[j]]) v3[j] = -1;
		for (int j = 0; j < 3; ++j)
		{
			points[idx++] = (v3[j] < 0) ? -1.f : vertices.data(v3[j] * 3)[0];
			points[idx++] = (v3[j] < 0) ? -1.f : vertices.data(v3[j] * 3)[1];
			points[idx++] = (v3[j] < 0) ? 0.0f : vertices.data(v3[j] * 3)[2];
		}
	}
}

snow::float3 FaceDB::face_norm(const Tensor3 &vertices, int ti)
{
	int k[3];
	k[0] = _triangles[ti][0] * 3;
	k[1] = _triangles[ti][1] * 3;
	k[2] = _triangles[ti][2] * 3;
    const double *p0 = vertices.data(k[0]);
    const double *p1 = vertices.data(k[1]);
    const double *p2 = vertices.data(k[2]);
	snow::float3 v0{
		(float)(p1[0] - p0[0]),
		(float)(p1[1] - p0[1]),
		(float)(p1[2] - p0[2])
	};
	snow::float3 v1{
		(float)(p2[0] - p1[0]),
		(float)(p2[1] - p1[1]),
		(float)(p2[2] - p1[2])
	};
	return snow::normalize(snow::cross(v0, v1));
};

void FaceDB::update_gl_normal(const Tensor3 &vertices, float *normal, bool is_points_masked)
{
	if (_v_norms.size() != n_vertices()) _v_norms.resize(n_vertices());
	if (_f_norms.size() != n_triangles()) _f_norms.resize(n_triangles());

	for (int i = 0; i < n_triangles(); ++i) {
		_f_norms[i] = (face_norm(vertices, i));
	}
	for (int vi = 0; vi < ndim_vert() / 3; ++vi) {
		snow::float3 norm{ 0, 0, 0 };
		for (int i = 0; i < _faces_of_point[vi].size(); ++i) {
			int f0 = _faces_of_point[vi][i] / 2 * 2;
			int f1 = _faces_of_point[vi][i] / 2 * 2 + 1;
			auto N = snow::dot(_f_norms[f0], _f_norms[f1]);
			norm = norm + _f_norms[f0];
		}
		norm = norm / (float)_faces_of_point[vi].size();
		_v_norms[vi] = snow::normalize(norm);
	}
	if (normal)
		for (int ti = 0; ti < n_triangles(); ++ti) {
			for (int i = 0; i < 3; ++i) {
				float *_n = &normal[ti * 9 + i * 3];
				if (is_points_masked)
				{
					_n[0] = _n[1] = 0; _n[2] = 1;
				}
				else
				{
					for (int j = 0; j < 3; ++j) {
						_n[j] = (&_v_norms[_triangles[ti][i]])[j];
					}
				}
			}
		}
}

snow::float3 FaceDB::vertex_norm(const Tensor3 &vertice, int id)
{
	int cnt = 0;
	snow::float3 norm{ 0, 0, 0 };
	for (int i = 0; i < _faces_of_point[id].size(); ++i) {
		norm = norm + face_norm(vertice, _faces_of_point[id][i]);
		cnt++;
	}
	norm = norm / (float)cnt;
	return snow::normalize(norm);
}


void FaceDB::save_bin(std::string dir)
{
	
}

void FaceDB::load_bin(std::string dir)
{

}