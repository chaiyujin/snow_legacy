#include "snow_arcball.h"
#include "snow_camera.h"

namespace snow {

    Arcball::Arcball(CameraBase *camera, bool manipulateCamera, float radius, glm::vec3 center)
        : mCamera(camera)
        , mIsCamera(manipulateCamera)
        , mRadiusOfHalfHeight(radius)
        , mCenter(center)
        , mQuatObject(glm::angleAxis(0.f, glm::vec3(0.f, 1.f, 0.f)))
        , mQuatCamera(glm::angleAxis(0.f, glm::vec3(0.f, 1.f, 0.f)))
        , mIsMoving(false)
        , mSpeed(1.0)
    {
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
}