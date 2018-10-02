#include "bilinear_model.h"

snow::MemoryArena Parameter::gArena;

BilinearModel::BilinearModel(size_t count)
    : Core(FaceDB::CoreTensor())
    , mTvi1List(0), mTv1eList(0)
    , mTv11List(0), mMeshList(0)
    , mParamScalePtr(new ScaleParameter)
    , mParamIdenPtr(new IdenParameter)
    , mParamPosePtrList(0)
    , mParamExprPtrList(0)
    , mCount(0)
    , mIsChild(false) {
    appendModel(count);
    prepareAllModel();
}
BilinearModel::BilinearModel(const BilinearModel &father, int startVertexIndex)
    : Core(FaceDB::CoreTensor())
    , mParamScalePtr(father.mParamScalePtr)
    , mParamIdenPtr(father.mParamIdenPtr)
    , mParamPosePtrList(father.mParamPosePtrList)
    , mParamExprPtrList(father.mParamExprPtrList)
    , mCount(father.mCount)
    , mIsChild(true) {
    for (size_t i = 0; i < mCount; ++i) {
        mTvi1List.emplace_back(father.mTvi1List[i], startVertexIndex * 3, 3);
        mTv1eList.emplace_back(father.mTv1eList[i], startVertexIndex * 3, 3);
        mTv11List.emplace_back(father.mTv11List[i], startVertexIndex * 3, 3);
        mMeshList.emplace_back(father.mMeshList[i], startVertexIndex * 3, 3);
    }
}
BilinearModel::~BilinearModel() {
    if (!mIsChild) {
        delete mParamScalePtr;
        delete mParamIdenPtr;
        for (size_t i = 0; i < mCount; ++i) {
            delete mParamPosePtrList[i];
            delete mParamExprPtrList[i];
        }
    }
}
void BilinearModel::appendModel(size_t count) {
    if (mIsChild) throw std::runtime_error("[BilinearModel]: appendModel() should not be child.\n");
    ++mCount;

    mTvi1List.emplace_back();
    mTv1eList.emplace_back();
    mTv11List.emplace_back();
    mMeshList.emplace_back();

    mParamPosePtrList.push_back(new PoseParameter);
    mParamExprPtrList.push_back(new ExprParameter);
}

void BilinearModel::prepareAllModel() {
    if (mIsChild) throw std::runtime_error("[BilinearModel]: appendModel() should not be child.\n");

    for (size_t i = 0; i < mCount; ++i) {
        mTvi1List[i].resize({ FaceDB::NumDimVert(), FaceDB::NumDimIden(), 1                   });
        mTv1eList[i].resize({ FaceDB::NumDimVert(), 1,                   FaceDB::NumDimExpr() });
        mTv11List[i].resize({ FaceDB::NumDimVert(), 1,                   1,                  });
        mMeshList[i].resize({ FaceDB::NumDimVert(), 1,                   1,                  });
    }

    mMorphModel.setIndices(FaceDB::Triangles());
    mMorphModel.resize(FaceDB::NumVertices());
}

#ifdef PARAMETER_FACS
void BilinearModel::updateExprOnCore(size_t i, const double *expr) {
    Core.mulVec<2>(ExprParameter::FACS2Expr(expr).data(), mTvi1List[i]);
}
void BilinearModel::updateExpr(size_t i, const double *expr) {
    mTv1eList[i].mulVec<2>(ExprParameter::FACS2Expr(expr).data(), mTv11List[i]);
}
#else
void BilinearModel::updateExprOnCore(size_t i, const double *expr) {
    Core.mulVec<2>(expr, mTvi1List[i]);
}
void BilinearModel::updateExpr(size_t i, const double *expr) {
    mTv1eList[i].mulVec<2>(expr, mTv11List[i]);
}
#endif

void BilinearModel::updateIdenOnCore(size_t i, const double *iden) {
    Core.mulVec<1>(iden, mTv1eList[i]);
}
void BilinearModel::updateIden(size_t i, const double *iden) {
    mTvi1List[i].mulVec<1>(iden, mTv11List[i]);
}
void BilinearModel::updateScale(size_t i, const double *scale) {
    mTv11List[i].mul(*scale, mMeshList[i]);
}
void BilinearModel::rotateYXZ(size_t index, const double *rotateYXZ) {
    double *data;
    glm::dmat4 rmat = glm::eulerAngleYXZ(rotateYXZ[0], rotateYXZ[1], rotateYXZ[2]);
    for (int i = 0; i < mMeshList[index].shape(0); i += 3) {
        data = mMeshList[index].data(i);
        auto q = rmat * glm::dvec4(data[0], data[1], data[2], 1);
        data[0] = q.x; data[1] = q.y; data[2] = q.z;
    }
}
void BilinearModel::translate(size_t index, const double *translate) {
    double *data;
    for (int i = 0; i < mMeshList[index].shape(0); i += 3) {
        data = mMeshList[index].data(i);
        data[0] += translate[0]; data[1] += translate[1]; data[2] += translate[2];
    }
}
void BilinearModel::transformMesh(size_t index, const glm::mat4 &extraTransform) {
    double *data;
    for (int i = 0; i < mMeshList[index].shape(0); i += 3) {
        data = mMeshList[index].data(i);
        auto q = extraTransform * glm::dvec4(data[0], data[1], data[2], 1);
        data[0] = q.x; data[1] = q.y; data[2] = q.z;
    }
}
void BilinearModel::updateMorphModel(size_t mesh_index) {
    size_t index = mesh_index;
    FaceDB::UpdateNormals(mMeshList[index]);
    auto &model = mMorphModel;
    double *data;
    for (int i = 0; i < FaceDB::NumVertices(); ++i) {
        data = mMeshList[index].data(i * 3);
        model.vertex(i).x = data[0];
        model.vertex(i).y = data[1];
        model.vertex(i).z = data[2];
        model.normal(i).x = FaceDB::VertNormals()[i].x;
        model.normal(i).y = FaceDB::VertNormals()[i].y;
        model.normal(i).z = FaceDB::VertNormals()[i].z;
        model.textureCoord(i).x = 0.f;
        model.textureCoord(i).y = 0.f;
    }
}
std::vector<snow::double3> BilinearModel::getMeshContourCands(size_t index) {
    std::vector<snow::double3> ret;
    for (size_t i = 0; i < FaceDB::Contours().size(); ++i) {
        for (size_t j = 0; j < FaceDB::Contours()[i].size(); ++j) {
            int vi = FaceDB::Contours()[i][j] * 3;
            ret.push_back({ *mMeshList[index].data(vi), *mMeshList[index].data(vi + 1), *mMeshList[index].data(vi + 2) });
        }
    }
    return ret;
}
std::vector<size_t> BilinearModel::getContourMeshIndex(const std::vector<size_t> &candidateIndex) {
    std::vector<size_t> cands, ret;
    for (size_t i = 0; i < FaceDB::Contours().size(); ++i)
        for (size_t j = 0; j < FaceDB::Contours()[i].size(); ++j)
            cands.push_back(FaceDB::Contours()[i][j]);
    for (size_t i: candidateIndex) {
        if (i < cands.size())
            ret.push_back(cands[i]);
    }
    return ret;
}