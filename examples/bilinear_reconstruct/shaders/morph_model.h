#pragma once
#define SNOW_MODULE_OPENGL
#include <snow.h>
#include "../depth_source/data.h"
#include "../facedb/tensor.h"
#include "../facedb/facedb.h"
#define VERT_CODE ""\
    "layout (location = 0) in vec3 aPos;                    "\
    "layout (location = 1) in vec3 aNorm;                   "\
    "layout (location = 2) in vec2 aTexCoord;               "\
    "out vec3 FragPos;                                      "\
    "out vec3 FragNormal;                                   "\
    "out vec2 FragTexCoord;                                 "\
    "uniform mat4 Normal;                                   "\
    "uniform mat4 Model;                                    "\
    "uniform mat4 View;                                     "\
    "uniform mat4 Proj;                                     "\
    "void main() {                                          "\
    "  gl_Position  = Proj * View * Model * vec4(aPos, 1.0);"\
    "  FragPos      = vec3(Model  * vec4(aPos, 1.0));       "\
    "  FragNormal   = vec3(Normal * vec4(aNorm, 0.0));      "\
    "  FragTexCoord = aTexCoord;                            "\
    "}                                                      "
#define FRAG_CODE ""\
    "in vec3  FragPos;                                      "\
    "in vec3  FragNormal;                                   "\
    "in vec2  FragTexCoord;                                 "\
    "out vec4 outFragColor;                                 "\
    "uniform vec3 LightPos;                                 "\
    "uniform vec3 Ambient;                                  "\
    "uniform vec3 Diffuse;                                  "\
    "void main() {                                          "\
    "    /* ambient */                                      "\
    "    vec3 ambientColor = Ambient;                       "\
    "    /* diffuse */                                      "\
    "    vec3 normal = normalize(FragNormal);               "\
    "    vec3 lightDir = normalize(LightPos - FragPos);     "\
    "    float diff = max(dot(normal, lightDir), 0);        "\
    "    vec3 diffuseColor = diff * Diffuse;                "\
    "    /* ligth */                                        "\
    "    vec3 result = (ambientColor + diffuseColor);       "\
    "    outFragColor = vec4(result, 1.0);                  "\
    "}"


class MorphModelShader : public snow::Shader {
private:
    float *             mPointsPtr;
    uint32_t *          mIndicePtr;
    int                 mNumVertice;
    int                 mNumTriangles;
    GLuint              mVAO, mVBO, mEBO;
public:
    MorphModelShader(int numVertice, int numTriangles)
        : snow::Shader()
        , mPointsPtr(nullptr)
        , mIndicePtr(nullptr)
        , mNumVertice(numVertice)
        , mNumTriangles(numTriangles)
        , mVAO(0), mVBO(0), mEBO(0) {
        this->buildFromCode(VERT_CODE, FRAG_CODE);
        // alloc
        mPointsPtr = new float[mNumVertice * 8];
        mIndicePtr = new uint32_t[mNumTriangles * 3];
        memset(mPointsPtr, 0, sizeof(float) * mNumVertice * 8);
        memset(mIndicePtr, 0, sizeof(float) * mNumTriangles * 3);
        // gen buffer 
        glGenVertexArrays(1, &mVAO);
        glGenBuffers(1, &mVBO);
        glGenBuffers(1, &mEBO);

        glBindVertexArray(mVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * mNumVertice, mPointsPtr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 3 * mNumTriangles, mIndicePtr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0); // vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
        glEnableVertexAttribArray(1); // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(2); // tex_coords
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

        // unbind VAO
        glBindVertexArray(0);
    }
    ~MorphModelShader() {
        delete[] mPointsPtr;
        delete[] mIndicePtr;
        if (mVBO) glDeleteBuffers(1, &mVBO);
        if (mEBO) glDeleteBuffers(1, &mEBO);
        if (mVAO) glDeleteVertexArrays(1, &mVAO);
    }

    void draw() {
        // draw mesh
        glBindVertexArray(mVAO);
        glDrawElements(GL_TRIANGLES, mNumTriangles * 3, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void updateTriangles(const std::vector<snow::int3> &triangles) {
        if (triangles.size() > mNumTriangles) throw std::runtime_error("too many triangles.");
        mNumTriangles = triangles.size();
        for (size_t i = 0; i < triangles.size(); ++i) {
            mIndicePtr[i * 3 + 0] = triangles[i].x;
            mIndicePtr[i * 3 + 1] = triangles[i].y;
            mIndicePtr[i * 3 + 2] = triangles[i].z;
        }
        glBindVertexArray(mVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t) * mNumTriangles * 3, mIndicePtr);
        glBindVertexArray(0);
    }

    template <typename T, typename U, typename W>
    void updateVertices(const T *vertice, const U *normal, const W *texcoord, size_t numVertice) {
        if (numVertice > mNumVertice) throw std::runtime_error("too many vertices");
        mNumVertice = numVertice;
        for (size_t i = 0; i < mNumVertice; ++i) {
            mPointsPtr[i * 8 + 0] = vertice[i * 3 + 0];
            mPointsPtr[i * 8 + 1] = vertice[i * 3 + 1];
            mPointsPtr[i * 8 + 2] = vertice[i * 3 + 2];
            mPointsPtr[i * 8 + 3] = (normal)? normal[i * 3 + 0] : 0.f;
            mPointsPtr[i * 8 + 4] = (normal)? normal[i * 3 + 1] : 0.f;
            mPointsPtr[i * 8 + 5] = (normal)? normal[i * 3 + 2] : 0.f;
            mPointsPtr[i * 8 + 6] = (texcoord)? texcoord[i * 2 + 0] : 0.f;
            mPointsPtr[i * 8 + 7] = (texcoord)? texcoord[i * 2 + 1] : 0.f;
        }
        glBindVertexArray(mVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * mNumVertice * 8, mPointsPtr);
        glBindVertexArray(0);
    }
};

#undef VERT_CODE
#undef FRAG_CODE