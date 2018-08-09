#pragma once
#include <vector>
#include <iostream>
// third-party
#include <glad/glad.h>
#include <SDL2/SDL.h>
// snow
#include "common.h"

namespace snow {
    /* Standard camera is: Right (1, 0, 0), Up (0, 1, 0), Front (0, 0, -1)
     * quatFromStandard() will calculate the quaterion from standard camera to current pose.
     * All camera keep `mQuatFromStandard` and `mCenter` and 'mDistance` to determine the position and pose.
     */
    class CameraBase {
    protected:
        glm::quat mQuatFromStandard;
        glm::vec3 mCenter;
        float     mDistance;
    private:
        // up, front, right is update by quat
        glm::vec3 mUp;
        glm::vec3 mFront;
        glm::vec3 mRight;
        // eye is calculated by center, distance and front
        glm::vec3 mEye;
    public:
        static glm::vec3 gStandardUp;
        static glm::vec3 gStandardFront;
        static glm::vec3 gStandardRight;

        CameraBase(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
        virtual ~CameraBase() {}
        const glm::vec3 &up()          const { return mUp;    }
        const glm::vec3 &front()       const { return mFront; }
        const glm::vec3 &right()       const { return mRight; }
        const glm::vec3 &eye()         const { return mEye;   }
        const glm::mat4 viewMatrix()   const { return glm::lookAt(mEye, mCenter, mUp); }
        void  rotate(const glm::quat &q);
        static glm::quat QuatFromStandard(const glm::vec3 &up, const glm::vec3 &front);
    };

    class Camera : public CameraBase {
    public:
        // constructor
        Camera(glm::vec3 eye   =glm::vec3(0.0f, 0.0f, 20.0f),
               glm::vec3 up    =glm::vec3(0.0f, 1.0f, 0.0f),
               glm::vec3 center=glm::vec3(0.f, 0.f, 0.f));

        float            zoom()        const { return mZoom;  }

        void             zoomIn(float deltaAngle=1.f)  { mZoom = std::max( 5.f, mZoom - std::max(0.f, deltaAngle)); }
        void             zoomOut(float deltaAngle=1.f) { mZoom = std::min(85.f, mZoom + std::max(0.f, deltaAngle)); }
        void             processMouseEvent(SDL_Event &event);

    private:
        Arcball    *mArcballPtr;
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
        // options
        float       mZoom;
        // for event
        int         mLastX, mLastY;
        int         mWidth, mHeight;
        glm::vec3   mLastPos;

        void _updateCameraVectors();
    };
}