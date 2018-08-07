#pragma once
#include <vector>
#include <iostream>
// third-party
#include <glad/glad.h>
#include <SDL2/SDL.h>
// snow
#include "snow_math.h"

namespace snow {
    class Camera {
    public:
        // constructor
        Camera(glm::vec3 position=glm::vec3(0.0f, 0.0f, 0.0f),
               glm::vec3 front=glm::vec3(0.f, 0.f, -1.f),
               glm::vec3 up=glm::vec3(0.0f, 1.0f, 0.0f),
               glm::quat quaternion=glm::angleAxis(0.f, glm::vec3(0.f, 1.f, 0.f)))
            : mPosition(position)
            , mFront(front)
            , mWolrdFront(front)
            , mWorldUp(up)
            , mQuaternion(quaternion)
            , mZoom(45.f)
            , mLastX(-1), mLastY(-1)
        {
            updateCameraVectors();
        }

        glm::mat4 getViewMatrix() {
            return glm::lookAt(mPosition, mPosition + mFront, mUp);
        }

        const glm::vec3 &up()          const { return mUp; }
        const glm::vec3 &front()       const { return mFront; }
        const glm::vec3 &position()    const { return mPosition; }
        const glm::quat &quaternion()  const { return mQuaternion; }
        float     zoom()        const { return mZoom; }

        void      zoomIn(float deltaAngle=1.f)  { mZoom = std::max( 5.f, mZoom - std::max(0.f, deltaAngle)); }
        void      zoomOut(float deltaAngle=1.f) { mZoom = std::min(85.f, mZoom + std::max(0.f, deltaAngle)); }
        void      processMouseEvent(SDL_Event &event) {
            if (event.type == SDL_MOUSEWHEEL) {
                mZoom += event.wheel.y;
                mZoom = std::min(85.f, std::max(5.f, mZoom));
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_MIDDLE) {
                mLastX = event.button.x;
                mLastY = event.button.y;
                mLastPos = mPosition;
                auto *p = SDL_GetWindowFromID(event.button.windowID);
                SDL_GetWindowSize(p, &mWidth, &mHeight);
            }
            else if (event.type == SDL_MOUSEMOTION && mLastX >= 0 && mLastY >= 0) {
                float dx = (float)(event.button.x - mLastX) / (float)mWidth * 3.f;
                float dy = (float)(event.button.y - mLastY) / (float)mHeight * 3.f;
                mPosition.x = mLastPos.x - dx * mRight.x;
                mPosition.y = mLastPos.y + dy * mUp.y;
            }
            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_MIDDLE) {
                mLastX = mLastY = -1;
                mLastPos = mPosition;
            }
        }

    private:
        // camera attributes
        glm::vec3   mPosition;
        glm::vec3   mFront;
        glm::vec3   mUp;
        glm::vec3   mRight;
        glm::vec3   mWolrdFront;
        glm::vec3   mWorldUp;
        // quat
        glm::quat   mQuaternion;
        // options
        float       mZoom;
        // for event
        int         mLastX, mLastY;
        int         mWidth, mHeight;
        glm::vec3   mLastPos;

        void updateCameraVectors() {
            mFront = glm::normalize(glm::rotate(mQuaternion, mWolrdFront));
            mRight = glm::normalize(glm::cross(mFront, mWorldUp));
            mUp    = glm::normalize(glm::cross(mRight, mFront));
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