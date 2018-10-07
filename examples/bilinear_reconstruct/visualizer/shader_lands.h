#pragma once
#include <snow.h>
#define VERT_CODE ""\
    "layout (location = 0) in vec3 aPos;"\
    "void main() {"\
    "  gl_Position = vec4(aPos, 1.0);"\
    "}"
#define FRAG_CODE ""\
    "out vec4 FragColor;"\
    "void main() {"\
    "    FragColor = vec4(0.0, 1.0, 0.0, 1.0); "\
    "}"

class LandmarksShader : public snow::Shader {
private:
    float * mPointsPtr;
    int     mNumPoints;
    int     mMaxPoints;
    float   mPointSize;
    GLuint  mVAO, mVBO;
public:
    LandmarksShader(int numPoints=75)
        : mPointsPtr(nullptr)
        , mNumPoints(numPoints)
        , mMaxPoints(numPoints)
        , mPointSize(2.0f)
        , mVAO(0), mVBO(0)
    {
        this->buildFromCode(VERT_CODE, FRAG_CODE);
        mPointsPtr = new float[mNumPoints * 3];
        memset(mPointsPtr, 0, sizeof(float) * 3 * mNumPoints);
        // buffers
		glGenBuffers(1, &mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mVBO);
		glBufferData(GL_ARRAY_BUFFER, 3 * mNumPoints * sizeof(float), mPointsPtr, GL_DYNAMIC_DRAW);
        // vao
        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);
        // location = 0, aPos
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
        glBindVertexArray(0);
    }

    ~LandmarksShader() {
        delete[] mPointsPtr;
        if (mVBO) glDeleteBuffers(1, &mVBO);
        if (mVAO) glDeleteVertexArrays(1, &mVAO);
    }

    void draw() {
        glBindVertexArray(mVAO); 
        glPointSize(mPointSize);
        glDrawArrays(GL_POINTS, 0, mNumPoints);
        glBindVertexArray(0);
    }

    void update2DLandmarks(const std::vector<snow::float2> &points) {
        if (points.size() > mMaxPoints) throw std::runtime_error("2d landmarks has too many points.");
        mNumPoints = (int)points.size();
        for (int i = 0; i < points.size(); ++i) {
            mPointsPtr[i * 3 + 0] = points[i].x;
            mPointsPtr[i * 3 + 1] = points[i].y;
            mPointsPtr[i * 3 + 2] = 0.f;
        }
        glBindVertexArray(mVAO); 
		glBindBuffer(GL_ARRAY_BUFFER, mVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0 * sizeof(float), sizeof(float) * mNumPoints * 3, mPointsPtr);
        glBindVertexArray(0);
    }

    void update3DLandmarks(const std::vector<snow::float3> &points) {
        if (points.size() > mNumPoints) throw std::runtime_error("3d landmarks has too many points.");
        mNumPoints = (int)points.size();
        for (int i = 0; i < points.size(); ++i) {
            mPointsPtr[i * 3 + 0] = points[i].x;
            mPointsPtr[i * 3 + 1] = points[i].y;
            mPointsPtr[i * 3 + 2] = points[i].z;
        }
        glBindVertexArray(mVAO); 
		glBindBuffer(GL_ARRAY_BUFFER, mVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0 * sizeof(float), sizeof(float) * mNumPoints * 3, mPointsPtr);
        glBindVertexArray(0);
    }

    int  numPoints()                        const { return mNumPoints; }
    void setPointSize(float size)                 { mPointSize = size; }
};

#undef VERT_CODE
#undef FRAG_CODE