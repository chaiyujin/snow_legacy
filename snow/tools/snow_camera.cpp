#include "snow_camera.h"
#include "snow_arcball.h"

namespace snow {

    glm::vec3 CameraBase::gOrigin        = glm::vec3(0, 0, 0);
    glm::vec3 CameraBase::gStandardUp    = glm::vec3(0, 1, 0);
    glm::vec3 CameraBase::gStandardFront = glm::vec3(0, 0, -1);
    glm::vec3 CameraBase::gStandardRight = glm::vec3(1, 0, 0);

    glm::quat CameraBase::QuatFromStandard(const glm::vec3 &up, const glm::vec3 &front) {
        auto q0 = snow::quatBetween(gStandardUp, up);
        auto f0 = glm::rotate(q0, gStandardFront);
        auto q1 = snow::quatBetween(f0, front);
        return glm::cross(q0, q1);
    }

    CameraBase::CameraBase(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
        mInitEye = eye;
        mInitCenter = center;
        mInitUp = up;
        this->_initialize();
    }

    void CameraBase::rotateAroundCenter(const glm::quat &q) {
        mQuatAroundCenter = glm::cross(mQuatAroundCenter, q);
        this->_updateCameraVectors();
    }
    
    void CameraBase::rotateAroundFront(const glm::quat &q) {
        mQuatAroundFront = glm::cross(mQuatAroundFront, q);
        this->_updateCameraVectors();
    }

    void CameraBase::setQuatAroundCenter(const glm::quat &q) {
        mQuatAroundCenter = q;
        this->_updateCameraVectors();
    }

    void CameraBase::setQuatAroundFront(const glm::quat &q) {
        mQuatAroundFront = q;
        this->_updateCameraVectors();
    }

    void CameraBase::_initialize() {
        mZoom     = 45.f;
        mCenter   = mInitCenter;
        mDistance = glm::distance(mInitCenter, mInitEye);
        // correct up
        mEye   = mInitEye;
        mUp    = mInitUp;
        mFront = glm::normalize(mCenter - mEye);
        mRight = glm::normalize(glm::cross(mFront, mUp));
        mUp    = glm::normalize(glm::cross(mRight, mFront));
        // get quat
        mQuatAroundCenter = snow::quatBetween(gStandardFront, mFront);
        mQuatAroundFront  = snow::quatBetween(glm::rotate(mQuatAroundCenter, gStandardFront + gStandardUp) - mFront, mUp);
        this->_updateCameraVectors();
    }
    
    void CameraBase::_updateCameraVectors() {
        auto front = glm::rotate(mQuatAroundCenter, gStandardFront);
        auto up    = glm::rotate(mQuatAroundCenter, gStandardFront + gStandardUp) - front;
        mUp        = glm::rotate(mQuatAroundFront,  up);
        mFront     = front;
        mRight     = glm::normalize(glm::cross(mFront, mUp));
        mUp        = glm::normalize(glm::cross(mRight, mFront));
        mEye       = mCenter - mDistance * mFront;
    }

    ArcballCamera::ArcballCamera(glm::vec3 eye, glm::vec3 up, glm::vec3 center)
        : CameraBase(eye, center, up)
        , mArcballPtr(nullptr)
        , mLastX(-1), mLastY(-1)
        , mSpeedZoom(1.f), mSpeedMove(5.f), mSpeedRotate(1.f)
    {
        mArcballPtr = new Arcball(this, true);
        mArcballPtr->setSpeed(mSpeedRotate);
    }

    void ArcballCamera::reset() {
        _initialize(); mArcballPtr->reset();
    }

    void ArcballCamera::setSpeedRotate(float speed)  { mSpeedRotate = speed; mArcballPtr->setSpeed(mSpeedRotate); }

    void ArcballCamera::processMouseEvent(SDL_Event &event) {
        if (event.type == SDL_MOUSEWHEEL) {
            mZoom += event.wheel.y * mSpeedZoom;
            mZoom = std::min(85.f, std::max(5.f, mZoom));
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_MIDDLE) {
            mLastX = event.button.x;
            mLastY = event.button.y;
            mLastPos = mCenter;
            auto *p = SDL_GetWindowFromID(event.button.windowID);
            SDL_GetWindowSize(p, &mWidth, &mHeight);
        }
        else if (event.type == SDL_MOUSEMOTION && mLastX >= 0 && mLastY >= 0) {
            float dx = (float)(event.button.x - mLastX) / (float)mWidth  * mSpeedMove;
            float dy = (float)(mLastY - event.button.y) / (float)mHeight * mSpeedMove;
            glm::vec3 delta(dx, dy, 0);
            delta = glm::rotate(mQuatAroundCenter, delta);
            mCenter = mLastPos - delta;
        }
        else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_MIDDLE && mLastX >= 0 && mLastY >= 0) {
            mLastX = mLastY = -1;
            mLastPos = mCenter;
        }
        mArcballPtr->processMouseEvent(event);
        this->setQuatAroundCenter(mArcballPtr->quaternion());
    }
}