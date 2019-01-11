#pragma once
#include <snow.h>
#define VERT_CODE ""\
    "layout (location = 0) in vec3 aPos;"\
    "layout (location = 1) in vec2 aTexCoord;"\
    "out vec2 TexCoord;"\
    "void main() {"\
    "  gl_Position = vec4(aPos, 1.0);"\
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

/**
 * Show image, and landmarks on image
 **/
class PointCloudShader;
class ImageShader : public snow::Shader {
private:
    friend class PointCloudShader;
    float * mPointsPtr;
    float   mPointSize;
    GLuint  mVAO, mVBO, mEBO;
    GLuint  mTextureID;
    GLenum  mTextureUnit;
    int     mWidth, mHeight;

public:
    ImageShader(GLenum textureUnit=GL_TEXTURE0)
        : Shader()
        , mPointsPtr(nullptr)
        , mPointSize(3.0)
        , mVAO(0), mVBO(0), mEBO(0)
        , mTextureID(0)
        , mTextureUnit(textureUnit)
        , mWidth(0), mHeight(0)
    {
        // compile from code
        this->buildFromCode(VERT_CODE, FRAG_CODE);
        // fill points with -1
        const int points = 4;
        mPointsPtr = new float[points * 5];
        for (int i = 0; i < points * 5; ++i) mPointsPtr[i] = -1.f;
        // first 4 points for image
        float vertices[] = {
            /*     aPos             aTexCoord */
             1.0f,  1.0f, 0.0f,        1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,        0.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,        0.0f, 1.0f,
             1.0f, -1.0f, 0.0f,        1.0f, 1.0f,
        };
        memcpy(mPointsPtr, vertices, sizeof(float) * 20);

        glGenBuffers(1, &mVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, points * 5 * sizeof(float), mPointsPtr, GL_DYNAMIC_DRAW);

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

        // ebo
        uint32_t indices[6] = {0, 1, 2, 0, 2, 3};
        glGenBuffers(1, &mEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint32_t), &indices[0], GL_DYNAMIC_DRAW);

        glBindVertexArray(0);
    }

    ~ImageShader() {
        delete[] mPointsPtr;
        if (mVBO)       glDeleteBuffers(1, &mVBO);
        if (mEBO)       glDeleteBuffers(1, &mEBO);
        if (mVAO)       glDeleteVertexArrays(1, &mVAO);
        if (mTextureID) glDeleteTextures(1, &mTextureID);
    }

    void draw() {
        if (mTextureID) {
            glActiveTexture(mTextureUnit);
            glBindTexture(GL_TEXTURE_2D, mTextureID);
            glBindVertexArray(mVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
        }
    }
        
    void uploadImage(const uint8_t *data, int w, int h, GLenum format, GLenum type = GL_UNSIGNED_BYTE) {
        // size mismatch, realloc texture
        if (w != mWidth || h != mHeight) {
            if (mTextureID) { glDeleteTextures(1, &mTextureID); mTextureID = 0; }
            mWidth = w; mHeight = h;
        }

        glActiveTexture(mTextureUnit);
        if (mTextureID == 0) {
            glGenTextures(1, &mTextureID);
            glBindTexture(  GL_TEXTURE_2D, mTextureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(   GL_TEXTURE_2D, 0, format, w, h, 0, format, type, data);
            glBindTexture(  GL_TEXTURE_2D, 0 );
        }
        else {
            glBindTexture(  GL_TEXTURE_2D, mTextureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, type, (void*)data);
            glBindTexture(  GL_TEXTURE_2D, 0 );
        }
    }
};

#undef VERT_CODE
#undef FRAG_CODE