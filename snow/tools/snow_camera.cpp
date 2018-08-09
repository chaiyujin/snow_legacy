#include "snow_camera.h"
#include "snow_arcball.h"

namespace snow {

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
        mCenter   = center;
        mDistance = glm::distance(center, eye);
        // setting
        mEye   = eye;
        mUp    = up;
        mFront = glm::normalize(mCenter - mEye);
        mRight = glm::normalize(glm::cross(mFront, mUp));
        mUp    = glm::normalize(glm::cross(mRight, mFront));
        // get quat
        mQuatFromStandard = CameraBase::QuatFromStandard(mUp, mFront);
    }

    

    Camera::Camera(glm::vec3 eye, glm::vec3 up, glm::vec3 center)
        : CameraBase(eye, center, up)
        , mArcballPtr(nullptr)
        , mOriginEye(eye)
        , mOriginUp(up)
        , mOriginCenter(center)
        , mZoom(45.f)
        , mLastX(-1), mLastY(-1)
    {
        mCenter = mOriginCenter;
        mEye    = mOriginEye;
        mUp     = mOriginUp;
        this->_updateCameraVectors();
        // after update vectors !
        mArcballPtr = new Arcball(this, true);
    }

    void Camera::processMouseEvent(SDL_Event &event) {
        // if (event.type == SDL_MOUSEWHEEL) {
        //     mZoom += event.wheel.y;
        //     mZoom = std::min(85.f, std::max(5.f, mZoom));
        // }
        // else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_MIDDLE) {
        //     mLastX = event.button.x;
        //     mLastY = event.button.y;
        //     mLastPos = mEye;
        //     auto *p = SDL_GetWindowFromID(event.button.windowID);
        //     SDL_GetWindowSize(p, &mWidth, &mHeight);
        // }
        // else if (event.type == SDL_MOUSEMOTION && mLastX >= 0 && mLastY >= 0) {
        //     float dx = (float)(event.button.x - mLastX) / (float)mWidth * 3.f;
        //     float dy = (float)(event.button.y - mLastY) / (float)mHeight * 3.f;
        //     mEye.x = mLastPos.x - dx * mRight.x;
        //     mEye.y = mLastPos.y + dy * mUp.y;
        // }
        // else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_MIDDLE && mLastX >= 0 && mLastY >= 0) {
        //     mLastX = mLastY = -1;
        //     mLastPos = mEye;
        // }
        mArcballPtr->processMouseEvent(event);
        mQuaternion = mArcballPtr->quaternion();
        this->_updateCameraVectors();
    }

    void Camera::_updateCameraVectors() {
        // arcball rotation
        mEye   = glm::rotate(mQuaternion, mOriginEye);
        mUp    = glm::rotate(mQuaternion, mOriginEye + mOriginUp) - mEye;
        // update
        mFront = glm::normalize(mOriginCenter - mEye);
        mRight = glm::normalize(glm::cross(mFront, mUp));
        mUp    = glm::normalize(glm::cross(mRight, mFront));
        // move
        mEye   = mEye + mCenter - mOriginCenter;
    }
}