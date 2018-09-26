#pragma once
#include <vector>
#include <snow.h>
#include "tensor.h"

#define PARAMETER_FACS
// #define USE_75_45

#ifndef USE_75_45
#define USE_50_25
#endif

typedef Eigen::Matrix<double, -1, -1, Eigen::RowMajor> MatrixRM;
typedef Eigen::Matrix<double, -1, -1, Eigen::ColMajor> MatrixCM;

class FaceDB
{
    FaceDB() {}
    ~FaceDB() {}
    static std::vector<int>                 gTensorShape;
    static std::vector<int>                 gOriginShape;
    static Tensor3                          gCoreTensor;
    static MatrixRM                         gIdenUT;
    static MatrixRM                         gExprUT;
    static std::vector<double>              gIdenSingular;
    static std::vector<double>              gExprSingular;
    /* static face information */
    static std::vector<snow::int3>          gTriangles;
    static std::vector<std::vector<int>>    gTrianglesOfPoint;
    static std::vector<int>                 gFaceVertices;
    static std::vector<snow::int3>          gFaceTriangles;
    /* static contour candidates */
    static std::vector<std::vector<int>>    gContourCandidates;
    /* norms */
    static std::vector<snow::float3>        gVertNorms;
    static std::vector<snow::float3>        gFaceNorms;
    static const std::vector<int>           gLandmarks73;
public:
#ifdef USE_75_45

    static const int LengthIdentity = 75;
#ifdef PARAMETER_FACS
    static const int LengthExpression = 47;
#else
    static const int LengthExpression = 45;
#endif

#else

    static const int LengthIdentity = 50;
#ifdef PARAMETER_FACS
    static const int LengthExpression = 47;
#else
    static const int LengthExpression = 25;
#endif

#endif

    static double MAX_ALLOWED_WEIGHT_RANGE;
    static int                                  NumDimVert()    { return gTensorShape[0];     }
    static int                                  NumDimIden()    { return gTensorShape[1];     }
    static int                                  NumDimExpr()    { return gTensorShape[2];     }
    static int                                  NumVertices()   { return gTensorShape[0] / 3; }
    static int                                  NumTriangles()  { return gTriangles.size();   }
    static const Tensor3 &                      CoreTensor()    { return gCoreTensor;         }

    static const MatrixRM &                     IdenUT()        { return gIdenUT; }
    static const MatrixRM &                     ExprUT()        { return gExprUT; }
    static const std::vector<double> &          IdenSingular()  { return gIdenSingular; }
    static const std::vector<double> &          ExprSingular()  { return gExprSingular; }

    static const std::vector<snow::int3> &      Triangles()     { return gTriangles; }
    static const std::vector<int>  &            FaceVertices()  { return gFaceVertices; }
    static const std::vector<snow::int3> &      FaceTriangles() { return gFaceTriangles; }

    static const std::vector<snow::float3> &    VertNormals()   { return gVertNorms; }
    static const std::vector<snow::float3> &    FaceNormals()   { return gFaceNorms; }
    
    static const std::vector<int> &             Landmarks73()   { return gLandmarks73; }
    static const std::vector<std::vector<int>>  Contours()      { return gContourCandidates; }

    static void         Initialize(std::string dir);
    static void         QueryCore(const double *iden, const double *expr, Tensor3 &result);
    // update face, vert normals together
    static void         UpdateNormals(const Tensor3 &vertices);
    // calculate on-fly
    static snow::float3 GetFaceNormal(const Tensor3 &vertices, int iFace);
    static snow::float3 GetVertNormal(const Tensor3 &vertices, int iVert);
};
