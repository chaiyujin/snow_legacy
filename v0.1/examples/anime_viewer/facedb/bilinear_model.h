#pragma once

#include "facedb.h"
#include "parameter.h"
#include "tensor.h"
#include <vector>

class BilinearModel {
private:
    const Tensor3 &                 Core;
    std::vector<Tensor3>            mTvi1List;
    std::vector<Tensor3>            mTv1eList;
    std::vector<Tensor3>            mTv11List;
    std::vector<Tensor3>            mMeshList;

    ScaleParameter *                mParamScalePtr;
    IdenParameter *                 mParamIdenPtr;
    std::vector<PoseParameter *>    mParamPosePtrList;
    std::vector<ExprParameter *>    mParamExprPtrList;

    size_t                          mCount;
    bool                            mIsChild;

public:
    static int NumVertices() { return FaceDB::NumVertices(); }

    BilinearModel();
    BilinearModel(const BilinearModel &father, int startVertexId);
    ~BilinearModel();

    /**
     * It takes lots of time to do copy construction for Tensor3
     * So, firstly append empty tensors.
     * Then, prepare them together.
     * */
    void appendModel(size_t count=1);
    void prepareAllModel();

    size_t size()                     const { return mCount;       }
    Tensor3 &mesh(size_t i=0)               { return mMeshList[i]; }
    Tensor3 &tv11(size_t i)                 { return mTv11List[i]; }
    Tensor3 &tv1e(size_t i)                 { return mTv1eList[i]; }
    Tensor3 &tvi1(size_t i)                 { return mTvi1List[i]; }
    const Tensor3 &mesh(size_t i=0)   const { return mMeshList[i]; }
    snow::double3 meshVertex(size_t iMesh, size_t iVert) { return { *mMeshList[iMesh].data((int)iVert * 3), *mMeshList[iMesh].data((int)iVert * 3+1), *mMeshList[iMesh].data((int)iVert * 3+2) }; }

    /**
     * There are two possible paths of applying parameters
     * 1. iden -> expr -> scale -> rotate -> translate
     * 2. expr -> iden -> scale -> rotate -> translate
     * Any extra transform can be done with transformMesh()
     * */

    /* path 1 start with */
    void updateIdenOnCore   (size_t i, const double *iden);
    void updateExpr         (size_t i, const double *expr);
    /* path 2 start with */
    void updateExprOnCore   (size_t i, const double *expr);
    void updateIden         (size_t i, const double *iden);
    /* following are shared */
    void updateScale        (size_t i, const double *scale);
    void rotateYXZ          (size_t i, const double *rotateYXZ);
    void translate          (size_t i, const double *translate);

    /* using self's parameters */
    void updateIdenOnCore   (size_t i)  { updateIdenOnCore(i, mParamIdenPtr->param());            }
    void updateExpr         (size_t i)  { updateExpr      (i, mParamExprPtrList[i]->param());     }
    void updateExprOnCore   (size_t i)  { updateExprOnCore(i, mParamExprPtrList[i]->param());     }
    void updateIden         (size_t i)  { updateIden      (i, mParamIdenPtr->param());            }
    void updateScale        (size_t i)  { updateScale     (i, mParamScalePtr->param());           }
    void rotateYXZ          (size_t i)  { rotateYXZ       (i, mParamPosePtrList[i]->rotateYXZ()); }
    void translate          (size_t i)  { translate       (i, mParamPosePtrList[i]->translate()); }
    /* update all parameter for mesh i */
    void updateMesh         (size_t i=0){ updateIdenOnCore(i); updateExpr(i); updateScale(i); rotateYXZ(i); translate(i); }

    void transformMesh      (size_t i, const glm::mat4 &extraTransform);
    void updateMorphModel   (size_t mesh_index);

    std::vector<snow::double3> getMeshContourCands(size_t i);
    std::vector<size_t>        getContourMeshIndex(const std::vector<size_t> &candidateIndex);
    std::vector<size_t>        getContourIndex(size_t i, const glm::dmat4 &PVM);

    ScaleParameter &        scaleParameter()              { return *mParamScalePtr;       }
    IdenParameter  &        idenParameter()               { return *mParamIdenPtr;        }
    PoseParameter  &        poseParameter(size_t i)       { return *mParamPosePtrList[i]; }
    ExprParameter  &        exprParameter(size_t i)       { return *mParamExprPtrList[i]; }
    const ScaleParameter &  scaleParameter()        const { return *mParamScalePtr;       }
    const IdenParameter  &  idenParameter()         const { return *mParamIdenPtr;        }
    const PoseParameter  &  poseParameter(size_t i) const { return *mParamPosePtrList[i]; }
    const ExprParameter  &  exprParameter(size_t i) const { return *mParamExprPtrList[i]; }

    void resetPrivateParameterAt(size_t i)  { mParamPosePtrList[i]->reset(); mParamExprPtrList[i]->reset(); }
    void resetSharedParameter()             { mParamScalePtr->reset(); mParamIdenPtr->reset(); }
    void resetAllParameter()                { resetSharedParameter(); for (size_t i = 0; i < mCount; ++i) resetPrivateParameterAt(i); }

};
