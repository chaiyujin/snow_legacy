#include "snow_arcball.h"
#include "snow_camera.h"

namespace snow {

    Arcball::Arcball(CameraBase *camera, bool manipulateCamera, float radius, glm::vec3 center)
        : mShader("../glsl/arc_vert.glsl", "../glsl/arc_frag.glsl")
        , mCamera(camera)
        , mIsCamera(manipulateCamera)
        , mRadiusOfHalfHeight(radius)
        , mCenter(center)
        , mQuatObject(glm::angleAxis(0.f, glm::vec3(0.f, 1.f, 0.f)))
        , mQuatCamera(glm::angleAxis(0.f, glm::vec3(0.f, 1.f, 0.f)))
        , mIsMoving(false)
        , mSpeed(1.0)
    {
        this->_generateArc();
        mQuatCamera = glm::inverse(mCamera->quatAroundCenter());
    }
    
    glm::quat Arcball::quaternion() {
        if (mIsCamera) {
            glm::quat ret = (mIsMoving)? glm::cross(mQuatCamera, mDelta) : mQuatCamera;
            ret = glm::inverse(ret);
            return ret;
        }
        else {
            return (mIsMoving)? glm::cross(mQuatObject, mDelta) : mQuatObject;
        }
    }

    glm::mat4 Arcball::_modelMatrix() {
        if (mIsCamera) {
            return glm::toMat4((mIsMoving)? glm::cross(mQuatCamera, mDelta) : mQuatCamera);
        }
        else {
            return glm::toMat4((mIsMoving)? glm::cross(mQuatObject, mDelta) : mQuatObject);
        }
    }


    void Arcball::_updateDelta() {
        if (!mIsMoving) return;
        
        auto _to_vec = [=](const glm::vec2 &p) -> glm::vec3 {
            auto pos = glm::vec3(p.x, p.y, mRadiusOfHalfHeight);
            pos = glm::rotate(glm::inverse(mQuatCamera), pos);
            return glm::normalize(pos - mCenter);
        };

        auto v0 = _to_vec(mPrevPos);
        auto v1 = _to_vec(mCurrPos);

        mDelta = snow::quatBetween(v0, v1, mSpeed);
    }

    void Arcball::processMouseEvent(SDL_Event &event) {
        auto get_pos = [=](int x, int y) -> glm::vec2 {            
            return glm::vec2( (x - mHalfWidth ) / (float)mHalfHeight,
                              (mHalfHeight - y) / (float)mHalfHeight );  // y is reversed
        };
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            int w, h;
            auto *p = SDL_GetWindowFromID(event.button.windowID);
            SDL_GetWindowSize(p, &w, &h);
            mHalfWidth  = w / 2;
            mHalfHeight = h / 2;
            mPrevPos = get_pos(event.button.x, event.button.y);
            mCurrPos = mPrevPos;
            mIsMoving = true;
            _updateDelta();
        }
        else if (event.type == SDL_MOUSEMOTION && mIsMoving) {
            mCurrPos = get_pos(event.button.x, event.button.y);
            _updateDelta();
        }
        else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT && mIsMoving) {
            if (mIsCamera)
                mQuatCamera = glm::cross(mQuatCamera, mDelta);
            else
                mQuatObject = glm::cross(mQuatObject, mDelta);
            mIsMoving = false;
        }
    }

    void Arcball::_generateArc() {
        mArcData.clear();
        mArcIndices.clear();
        float r = mRadiusOfHalfHeight;
        auto sample_around = [=](glm::vec3 first, glm::vec3 axis, glm::vec3 rgb) -> std::vector<glm::vec3> {
            std::vector<glm::vec3> ret;
            int   N = SampleNumber;
            float deltaAngle = 2.0 * M_PI / N;
            for (int i = 0; i < N; ++i) {
                auto p = glm::rotate(glm::angleAxis(deltaAngle * i, axis), first);
                ret.push_back(p);
                ret.push_back(rgb);
            }
            return ret;
        };
        auto indices = [=](int count) -> std::vector<uint32_t> {
            std::vector<uint32_t> ret;
            int N = SampleNumber;
            for (int i = 0; i < N; ++i) {
                ret.push_back(i           + count * N);
                ret.push_back((i + 1) % N + count * N);
            }
            return ret;
        };

        auto sample0 = sample_around(glm::vec3(0, r, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 0));  // x
        auto sample1 = sample_around(glm::vec3(r, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));  // y
        auto sample2 = sample_around(glm::vec3(0, r, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1));  // z
        auto indices0 = indices(0);
        auto indices1 = indices(1);
        auto indices2 = indices(2);

        mArcData.insert(mArcData.end(), sample0.begin(), sample0.end());
        mArcData.insert(mArcData.end(), sample1.begin(), sample1.end());
        mArcData.insert(mArcData.end(), sample2.begin(), sample2.end());
        mArcIndices.insert(mArcIndices.end(), indices0.begin(), indices0.end());
        mArcIndices.insert(mArcIndices.end(), indices1.begin(), indices1.end());
        mArcIndices.insert(mArcIndices.end(), indices2.begin(), indices2.end());

        // gen buffer 
        glGenVertexArrays(1, &mVAO);
        glGenBuffers(1, &mVBO);
        glGenBuffers(1, &mEBO);

        glBindVertexArray(mVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, mArcData.size() * sizeof(glm::vec3), &mArcData[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mArcIndices.size() * sizeof(uint32_t), &mArcIndices[0], GL_STATIC_DRAW);

        // vertex position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)0);
        // tex_coords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)sizeof(glm::vec3));
        // unbind VAO
        glBindVertexArray(0);
    }

    void Arcball::draw(const glm::mat4 &project) {
        // draw
        mShader.use();
        if (mIsCamera) {
            glm::mat4 mvp = project * this->mCamera->viewMatrix() *
                            glm::translate(glm::mat4(1.0), this->mCamera->center());
            mShader.setMat4("model", mvp);
        }
        glBindVertexArray(mVAO);
        glDrawElements(GL_LINES, mArcIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}