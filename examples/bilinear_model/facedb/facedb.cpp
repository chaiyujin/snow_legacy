#include "facedb.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

double                          FaceDB::MAX_ALLOWED_WEIGHT_RANGE = 1.25;
std::vector<int>                FaceDB::gTensorShape;
std::vector<int>                FaceDB::gOriginShape;
Tensor3                         FaceDB::gCoreTensor;
std::vector<double>             FaceDB::gIdenSingular;
std::vector<double>             FaceDB::gExprSingular;
MatrixRM                        FaceDB::gIdenUT;
MatrixRM                        FaceDB::gExprUT;
/* static face information */
std::vector<snow::int3>         FaceDB::gTriangles;
std::vector<std::vector<int>>   FaceDB::gTrianglesOfPoint;
std::vector<int>                FaceDB::gFaceVertices;
std::vector<snow::int3>         FaceDB::gFaceTriangles;
/* static contour candidates */
std::vector<std::vector<int>>   FaceDB::gContourCandidates;
/* norms */
std::vector<snow::float3>       FaceDB::gVertNorms;
std::vector<snow::float3>       FaceDB::gFaceNorms;


const std::vector<int> FaceDB::gLandmarks73({
    2580, 9759, 2573, 2748, 6740, 6770, 6541, 3611, 10573, 3865, 3854, 3842, 10652, 5500, 11284, 10751, 4247, 712, 709, 4245, 4248, 9251, 2136, 7168, 2134, 2123, 2124, 1965, 2171, 7100, 9445, 576, 4349, 10820, 4392, 9236, 6335, 1789, 6388, 6274, 10459, 10492, 283, 10733, 8994, 10453, 6150, 1620, 6090, 8814, 182, 3237, 174, 10306, 3231, 3190, 6117, 8864, 6164, 3272, 3275, 10334, 3280, 6144, 8940, 9310, 9447, 9443, 2170, 4339, 10880, 4356, 10687 
});

void FaceDB::Initialize(std::string dir) {
    std::string tensor_path = snow::path::Join(dir, "tensor.bin");
    std::string face_path   = snow::path::Join(dir, "triangles.txt");
    std::string cont_path   = snow::path::Join(dir, "contourpoints.txt");
    std::string mask_path   = snow::path::Join(dir, "face_mask.txt");
    std::string iden_s_path = snow::path::Join(dir, "iden_singular.txt");
    std::string expr_s_path = snow::path::Join(dir, "expr_singular.txt");

    if (!snow::path::AllExists({ tensor_path, face_path, cont_path, mask_path, iden_s_path, expr_s_path })) {
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
        gTensorShape.resize(3);
        gOriginShape.resize(3);
        size_t read_size;
        read_size = fread(gTensorShape.data(), sizeof(int), 3, fp_tensor);
        read_size = fread(gOriginShape.data(), sizeof(int), 3, fp_tensor);
        int size = gTensorShape[0] * gTensorShape[1] * gTensorShape[2];
        tmp_alloc_read(&tmp, &fp_tensor, size);
        // unfold core tensor
        gCoreTensor.resize(gTensorShape);
        gCoreTensor.unfoldData<float>(tmp, 1);
        /* iden trans */
        size = gTensorShape[1] * gOriginShape[1];
        tmp_alloc_read(&tmp, &fp_tensor, size);
        gIdenUT.resize(gTensorShape[1], gOriginShape[1]);
        for (int i = 0; i < gTensorShape[1]; ++i)
            for (int j = 0; j < gOriginShape[1]; ++j)
                gIdenUT(i, j) = tmp[i * gOriginShape[1] + j];
        /* expr trans */
        size = gTensorShape[2] * gOriginShape[2];
        tmp_alloc_read(&tmp, &fp_tensor, size);
        gExprUT.resize(gTensorShape[2], gOriginShape[2]);
        for (int i = 0; i < gTensorShape[2]; ++i)
            for (int j = 0; j < gOriginShape[2]; ++j)
                gExprUT(i, j) = tmp[i * gOriginShape[2] + j];
        /* close */
        delete[] tmp;
        fclose(fp_tensor);

        // std::cout << " core  shape: " << gTensorShape << std::endl;
        // std::cout << "origin shape: " << gOriginShape << std::endl;
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

            gTensorShape.resize(3);
            gOriginShape.resize(3);
            gTensorShape[0] = n;  gOriginShape[0] = n;
            gTensorShape[1] = l; gOriginShape[1] = 150;
            gTensorShape[2] = m; gOriginShape[2] = 47;

            // Tensor3 tmp;
            gCoreTensor.resize({ n, l, m });
            gCoreTensor.unfoldData<double>(tmp_tensor, 2);

            // tmp.mul(-1, gCoreTensor);

            std::cout << " core  shape: " << gTensorShape << std::endl;
            std::cout << "origin shape: " << gOriginShape << std::endl;
            delete[] tmp_tensor;
        }

        {
            Eigen::MatrixXd sigma_Wid;
            std::fstream fin;
            fin.open(prior_id, std::ios::in | std::ios::binary);

            int ndims;
            fin.read(reinterpret_cast<char*>(&ndims), sizeof(int));
            std::cout << "identity prior dim = " << ndims << std::endl;

            // _iden_avg.resize(ndims);
            // _iden_0.resize(ndims);
            // sigma_Wid.resize(ndims, ndims);

            // fin.read(reinterpret_cast<char*>(_iden_avg.data()), sizeof(double)*ndims);
            // fin.read(reinterpret_cast<char*>(_iden_0.data()), sizeof(double)*ndims);
            // fin.read(reinterpret_cast<char*>(sigma_Wid.data()), sizeof(double)*ndims*ndims);
            // _iden_inv_sigma = sigma_Wid.inverse();
            // // Take the diagonal
            // _iden_inv_sigma_diag = Eigen::VectorXd(ndims);
            // for (int i = 0; i<ndims; ++i) {
            //     _iden_inv_sigma_diag(i) = _iden_inv_sigma(i, i);
            // }

            fin.seekg(sizeof(double)*ndims*(ndims+2), std::ios::cur);

            int m, n;
            fin.read(reinterpret_cast<char*>(&m), sizeof(int));
            fin.read(reinterpret_cast<char*>(&n), sizeof(int));
            std::cout << "iden_UT size: " << n << 'x' << m << std::endl;
            gIdenUT.resize(n, m);
            fin.read(reinterpret_cast<char*>(gIdenUT.data()), sizeof(double)*m*n);


            for (int i = 0; i < gIdenUT.rows(); ++i)
            {
                for (int j = 0; j < gIdenUT.cols(); ++j)
                {
                    if (abs(gIdenUT(i, j)) < 1e-30)
                        gIdenUT(i, j) = 0;
                }
            }


            // _iden_max.resize(n);
            // _iden_min.resize(n);
            // for (int i = 0; i<n; ++i) {
            //     _iden_max(i) = _iden_avg(i) + (gIdenUT.row(i).maxCoeff() - _iden_avg(i)) * MAX_ALLOWED_WEIGHT_RANGE;
            //     _iden_min(i) = _iden_avg(i) + (gIdenUT.row(i).minCoeff() - _iden_avg(i)) * MAX_ALLOWED_WEIGHT_RANGE;
            // }

            fin.close();
        }

        {
            Eigen::MatrixXd sigma_Wexp;
            std::fstream fin;
            fin.open(prior_ex, std::ios::in | std::ios::binary);

            int ndims;
            fin.read(reinterpret_cast<char*>(&ndims), sizeof(int));
            std::cout << "expr prior dim = " << ndims << std::endl;

            // _expr_avg.resize(ndims);
            // _expr_0.resize(ndims);
            // sigma_Wexp.resize(ndims, ndims);

            // fin.read(reinterpret_cast<char*>(_expr_avg.data()), sizeof(double)*ndims);
            // fin.read(reinterpret_cast<char*>(_expr_0.data()), sizeof(double)*ndims);
            // fin.read(reinterpret_cast<char*>(sigma_Wexp.data()), sizeof(double)*ndims*ndims);
            // _expr_inv_sigma = sigma_Wexp.inverse();
            // // Take the diagonal
            // _expr_inv_sigma_diag = Eigen::VectorXd(ndims);
            // for (int i = 0; i<ndims; ++i) {
            //     _expr_inv_sigma_diag(i) = _expr_inv_sigma(i, i);
            // }

            fin.seekg(sizeof(double)*ndims*(ndims+2), std::ios::cur);

            int m, n;
            fin.read(reinterpret_cast<char*>(&m), sizeof(int));
            fin.read(reinterpret_cast<char*>(&n), sizeof(int));
            std::cout << "expr_UT size: " << n << 'x' << m << std::endl;
            gExprUT.resize(n, m);
            fin.read(reinterpret_cast<char*>(gExprUT.data()), sizeof(double)*m*n);

            // _expr_max.resize(n);
            // _expr_min.resize(n);
            // for (int i = 0; i<n; ++i) {
            //     _expr_max(i) = _expr_avg(i) + (gExprUT.row(i).maxCoeff() - _expr_avg(i)) * MAX_ALLOWED_WEIGHT_RANGE;
            //     _expr_min(i) = _expr_avg(i) + (gExprUT.row(i).minCoeff() - _expr_avg(i)) * MAX_ALLOWED_WEIGHT_RANGE;
            // }

            fin.close();
        }
    }
#endif


    /* read faces */
    {
        int tmp_vi[4], tmp_uvi[4], tmp_ni[4];
        FILE *fp_faces = fopen(face_path.c_str(), "r");
        gTrianglesOfPoint.clear();
        gTriangles.clear();
        gTrianglesOfPoint.resize(gTensorShape[0] / 3);
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
            gTriangles.push_back({ tmp_vi[0], tmp_vi[1], tmp_vi[2] });
            gTrianglesOfPoint[tmp_vi[0]].push_back((int)gTriangles.size() - 1);
            gTrianglesOfPoint[tmp_vi[1]].push_back((int)gTriangles.size() - 1);
            gTrianglesOfPoint[tmp_vi[2]].push_back((int)gTriangles.size() - 1);
            gTriangles.push_back({ tmp_vi[0], tmp_vi[2], tmp_vi[3] });
            gTrianglesOfPoint[tmp_vi[0]].push_back((int)gTriangles.size() - 1);
            gTrianglesOfPoint[tmp_vi[2]].push_back((int)gTriangles.size() - 1);
            gTrianglesOfPoint[tmp_vi[3]].push_back((int)gTriangles.size() - 1);
            // printf("%d %d %d %d\n", tmp_vi[0], tmp_vi[1], tmp_vi[2], tmp_vi[3]);
        }
        fclose(fp_faces);
        // printf("read face done\n");
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
        gContourCandidates.resize(lines.size());

        std::transform(lines.begin(), lines.end(), gContourCandidates.begin(),
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
        // printf("read contour done\n");
    }
    /* read singular values */
    {
        {
            std::ifstream fin(iden_s_path);
            double x;
            gIdenSingular.clear();
            while (fin >> x)
            {
                gIdenSingular.push_back(x);
            }
            fin.close();
        }
        {
            std::ifstream fin(expr_s_path);
            double x;
            gExprSingular.clear();
            while (fin >> x)
            {
                gExprSingular.push_back(x);
            }
            fin.close();
        }
        // printf("read expr singular done\n");
    }
    /* read face only mesh */
    {
        std::ifstream fin(mask_path);
        if (fin.is_open())
        {
            gFaceVertices.clear();
            gFaceTriangles.clear();
            int n;
            fin >> n;
            for (int i = 0; i < n; ++i)
            {
                int id;
                fin >> id;
                gFaceVertices.push_back(id);
            }
            fin >> n;
            for (int i = 0; i < n; ++i)
            {
                int x, y, z;
                fin >> x >> y >> z;
                gFaceTriangles.push_back({ x, y, z });
            }
            fin.close();
        }
        // printf("read face only mesh done\n");
    }
}

void FaceDB::QueryCore(const double *iden, const double *expr, Tensor3 &result) {
    gCoreTensor.mulVec(iden, expr, result);
}

void FaceDB::UpdateNormals(const Tensor3 &vertices)
{
    if (gVertNorms.size() != NumVertices())  gVertNorms.resize(NumVertices());
    if (gFaceNorms.size() != NumTriangles()) gFaceNorms.resize(NumTriangles());

    for (int i = 0; i < NumTriangles(); ++i) {
        gFaceNorms[i] = (GetFaceNormal(vertices, i));
    }

    for (int vi = 0; vi < NumVertices(); ++vi) {
        snow::float3 norm{ 0, 0, 0 };
        for (int i = 0; i < gTrianglesOfPoint[vi].size(); ++i) {
            int f0 = gTrianglesOfPoint[vi][i];
            norm += gFaceNorms[f0];
        }
        norm = norm / (float)gTrianglesOfPoint[vi].size();
        gVertNorms[vi] = snow::normalize(norm);
    }
}

snow::float3 FaceDB::GetFaceNormal(const Tensor3 &vertices, int ti) {
    const int k0 = gTriangles[ti][0] * 3;
    const int k1 = gTriangles[ti][1] * 3;
    const int k2 = gTriangles[ti][2] * 3;
    snow::float3 v0 = { 
        (float)(*vertices.data(k1 + 0) - *vertices.data(k0 + 0)),
        (float)(*vertices.data(k1 + 1) - *vertices.data(k0 + 1)),
        (float)(*vertices.data(k1 + 2) - *vertices.data(k0 + 2))
    };
    snow::float3 v1 = {
        (float)(*vertices.data(k2 + 0) - *vertices.data(k1 + 0)),
        (float)(*vertices.data(k2 + 1) - *vertices.data(k1 + 1)),
        (float)(*vertices.data(k2 + 2) - *vertices.data(k1 + 2))
    };
    return snow::normalize(snow::cross(v0, v1));
}

snow::float3 FaceDB::GetVertNormal(const Tensor3 &vertice, int id) {
    int cnt = 0;
    snow::float3 norm = { 0, 0, 0 };
    for (int i = 0; i < gTrianglesOfPoint[id].size(); ++i) {
        norm += GetFaceNormal(vertice, gTrianglesOfPoint[id][i]);
        cnt++;
    }
    norm = norm / (float)cnt;
    return snow::normalize(norm);
}
