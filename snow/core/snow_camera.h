#pragma once
#include <vector>
#include <iostream>
// third-party
#include <glad/glad.h>
#include <SDL2/SDL.h>
// snow
#include "snow_math.h"
#include "../gui/snow_arcball.h"

namespace snow {
    class Camera {
    public:
        // constructor
        Camera(glm::vec3 eye=glm::vec3(0.0f, 0.0f, 20.0f),
               glm::vec3 up=glm::vec3(0.0f, 1.0f, 0.0f),
               glm::vec3 center=glm::vec3(0.f, 0.f, 0.f))
            : mArcball(true)
            , mOriginEye(eye)
            , mOriginUp(up)
            , mOriginCenter(center)
            , mQuaternion(glm::angleAxis(0.f, glm::vec3(0.f, 1.f, 0.f)))
            , mZoom(45.f)
            , mLastX(-1), mLastY(-1)
        {
            mCenter = mOriginCenter;
            mEye    = mOriginEye;
            mUp     = mOriginUp;
            updateCameraVectors();
        }

        glm::mat4 getViewMatrix() {
            return glm::lookAt(mEye, mCenter, mUp);
        }

        const glm::vec3 &up()          const { return mUp; }
        const glm::vec3 &front()       const { return mFront; }
        const glm::vec3 &position()    const { return mEye; }
        const glm::quat &quaternion()  const { return mQuaternion; }
        float     zoom()        const { return mZoom; }

        void      zoomIn(float deltaAngle=1.f)  { mZoom = std::max( 5.f, mZoom - std::max(0.f, deltaAngle)); }
        void      zoomOut(float deltaAngle=1.f) { mZoom = std::min(85.f, mZoom + std::max(0.f, deltaAngle)); }
        void      processMouseEvent(SDL_Event &event) {
            // if (event.type == SDL_MOUSEWHEEL) {
            //     mZoom += event.wheel.y;
            //     mZoom = std::min(85.f, std::max(5.f, mZoom));
            // }
            // else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_MIDDLE) {
            //     mLastX = event.button.x;
            //     mLastY = event.button.y;
            //     mLastPos = mPosition;
            //     auto *p = SDL_GetWindowFromID(event.button.windowID);
            //     SDL_GetWindowSize(p, &mWidth, &mHeight);
            // }
            // else if (event.type == SDL_MOUSEMOTION && mLastX >= 0 && mLastY >= 0) {
            //     float dx = (float)(event.button.x - mLastX) / (float)mWidth * 3.f;
            //     float dy = (float)(event.button.y - mLastY) / (float)mHeight * 3.f;
            //     mPosition.x = mLastPos.x - dx * mRight.x;
            //     mPosition.y = mLastPos.y + dy * mUp.y;
            // }
            // else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_MIDDLE && mLastX >= 0 && mLastY >= 0) {
            //     mLastX = mLastY = -1;
            //     mLastPos = mPosition;
            // }
            mArcball.processMouseEvent(event);
            mQuaternion = mArcball.quaternion();
            updateCameraVectors();
        }

    private:
        Arcball     mArcball;
        // origin
        glm::vec3   mOriginEye;
        glm::vec3   mOriginUp;
        glm::vec3   mOriginCenter;
        // camera attributes
        glm::vec3   mEye;
        glm::vec3   mUp;
        glm::vec3   mCenter;
        glm::vec3   mFront;
        glm::vec3   mRight;
        // quat
        glm::quat   mQuaternion;
        // options
        float       mZoom;
        // for event
        int         mLastX, mLastY;
        int         mWidth, mHeight;
        glm::vec3   mLastPos;

        void updateCameraVectors() {
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
    };

    inline std::ostream &operator<<(std::ostream &out, const Camera &camera) {
        out << "Camera: { "
            << "'position': " << camera.position() << ", "
            << "'front': " << camera.front() << ", "
            << "'up': " << camera.up() << " }";
        return out;
    }
}