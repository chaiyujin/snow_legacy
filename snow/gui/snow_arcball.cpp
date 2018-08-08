#include "snow_arcball.h"

namespace snow {

    Arcball::Arcball(bool isCamera, float radius, glm::vec3 center)
        : mIsCamera(isCamera)
        , mRadiusOfHalfHeight(radius)
        , mCenter(center)
        , mQuat(glm::angleAxis(0.f, glm::vec3(0.f, 1.f, 0.f)))
        , mIsMoving(false)
    {}
    
    glm::quat Arcball::quaternion() { 
        glm::quat ret = (mIsMoving)? glm::cross(mQuat, mDelta) : mQuat;
        if (mIsCamera)
            ret = glm::inverse(ret);
        return ret;
    }

    void Arcball::_updateDelta() {
        if (!mIsMoving) return;
        auto _to_vec = [=](const glm::vec2 &p) -> glm::vec3 {
            auto pos = glm::vec3(p.x, p.y, mRadiusOfHalfHeight);
            pos = glm::rotate(glm::inverse(mQuat), pos);
            return glm::normalize(pos - mCenter);
        };
        auto v0 = _to_vec(mPrevPos);
        auto v1 = _to_vec(mCurrPos);

        float angle = std::acos(glm::dot(v0, v1));
        if (std::isnan(angle)) angle = 0.f;
        glm::vec3 axis = (glm::all(glm::equal(v0, v1)))
                       ? glm::vec3(0, 1, 0)
                       : glm::normalize(glm::cross(v0, v1));
        mDelta = glm::angleAxis(angle, axis);
    }

    void Arcball::processMouseEvent(SDL_Event &event) {
        auto get_pos = [=](int x, int y) -> glm::vec2 {
            
            auto ret = glm::vec2( (x - mHalfWidth ) / (float)mHalfHeight,
                                  (y - mHalfHeight) / (float)mHalfHeight );
            // std::cout << ret.x << " " << ret.y << std::endl;
            return ret;
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
            mQuat = glm::cross(mQuat, mDelta);
            mIsMoving = false;
        }
    }
}