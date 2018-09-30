#pragma once
#include <snow.h>
#include "image.h"
#include "../depth_source/data.h"
#define VERT_CODE ""\
    "layout (location = 0) in vec3 aPos;"\
    "layout (location = 1) in vec2 aTexCoord;"\
    "out vec2 TexCoord;"\
    "uniform mat4 Model;"\
    "uniform mat4 View;"\
    "uniform mat4 Proj;"\
    "void main() {"\
    "  gl_Position = Proj * View * Model * vec4(aPos, 1.0);"\
    "  TexCoord = aTexCoord;"\
    "}"
#define FRAG_CODE ""\
    "in vec2 TexCoord;"\
    "out vec4 FragColor;"\
    "uniform sampler2D ImageTexture;"\
    "void main() {"\
    "    if (TexCoord[0] < 0) FragColor = vec4(0.0, 1.0, 0.0, 1.0);"\
    "    else                 FragColor = texture(ImageTexture, TexCoord);"\
    "}"

class PointCloudShader : public snow::Shader {
private:
    float *             mPointsPtr;
    int                 mNumPoints;
    float               mPointSize;
    GLuint              mVAO, mVBO, mEBO;
    const ImageShader * mImageShader;
public:
    PointCloudShader(int numPoints, const ImageShader *textureShader)
        : Shader()
        , mPointsPtr(nullptr)
        , mNumPoints(numPoints)
        , mPointSize(1.0)
        , mVAO(0), mVBO(0), mEBO(0)
        , mImageShader(textureShader) {
        this->buildFromCode(VERT_CODE, FRAG_CODE);
        mPointsPtr = new float[mNumPoints * 5];
        memset(mPointsPtr, 0, sizeof(float) * 5 * mNumPoints);
        // buffers
		glGenBuffers(1, &mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mVBO);
		glBufferData(GL_ARRAY_BUFFER, 5 * mNumPoints * sizeof(float), mPointsPtr, GL_DYNAMIC_DRAW);
        // vao
        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);
        // location = 0, aPos
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
        // location = 1, aTexCoord
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glBindVertexArray(0);
    }

    ~PointCloudShader() {
        delete[] mPointsPtr;
        if (mVBO) glDeleteBuffers(1, &mVBO);
        if (mEBO) glDeleteBuffers(1, &mEBO);
        if (mVAO) glDeleteVertexArrays(1, &mVAO);
    }

    void draw() {
        glActiveTexture(mImageShader->mTextureUnit);
        if (mImageShader->mTextureID) glBindTexture(GL_TEXTURE_2D, mImageShader->mTextureID);
        glBindVertexArray(mVAO); 
        glPointSize(mPointSize);
        glDrawArrays(GL_POINTS, 0, mNumPoints);
        glBindVertexArray(0);
        if (mImageShader->mTextureID) glBindTexture(GL_TEXTURE_2D, 0);
    }

    void updateWithPointCloud(const PointCloud &points) {
        if (points.size() > mNumPoints) throw std::runtime_error("point cloud has too many points.");
        mNumPoints = points.size();
        const auto &vertice = points.verticeList();
        const auto &texture = points.textureCoordList();
        for (size_t i = 0; i < points.size(); ++i) {
            mPointsPtr[i * 5 + 0] = vertice[i].x;
            mPointsPtr[i * 5 + 1] = vertice[i].y;
            mPointsPtr[i * 5 + 2] = vertice[i].z;
            mPointsPtr[i * 5 + 3] = texture[i].x;
            mPointsPtr[i * 5 + 4] = texture[i].y;
        }
        glBindVertexArray(mVAO); 
		glBindBuffer(GL_ARRAY_BUFFER, mVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0 * sizeof(float), sizeof(float) * mNumPoints * 5, mPointsPtr);
        glBindVertexArray(0);
    }
};

#undef VERT_CODE
#undef FRAG_CODE