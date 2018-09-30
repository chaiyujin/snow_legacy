#pragma once
#include <vector>
#include <iostream>
// third-party
#include <glad/glad.h>
#include <SDL2/SDL.h>
// snow
#include "common.h"

namespace snow {
    /**
     *  Standard camera is: Right (1, 0, 0), Up (0, 1, 0), Front (0, 0, -1)
     *
     *    A camera firstly rotate around it's center (also called lookat),
     *    then rotate around it's front axis.
     *    The eye (also called position) = center - distance * front
     * 
     **/
    class CameraBase {
    protected:
        glm::quat mQuatAroundFront;
        glm::quat mQuatAroundCenter;
        glm::vec3 mCenter;
        float     mDistance;
        float     mZoom;
        void _updateCameraVectors();
        void _initialize();
    private:
        // up, front, right is update by quat
        glm::vec3 mUp;
        glm::vec3 mFront;
        glm::vec3 mRight;
        // eye is calculated by center, distance and front
        glm::vec3 mEye;

        // initial
        glm::vec3 mInitEye, mInitCenter, mInitUp;

    public:
        static glm::vec3 gOrigin;
        static glm::vec3 gStandardUp;
        static glm::vec3 gStandardFront;
        static glm::vec3 gStandardRight;

        CameraBase(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
        virtual ~CameraBase() {}
        const glm::vec3 &up()          const { return mUp;    }
        const glm::vec3 &front()       const { return mFront; }
        const glm::vec3 &right()       const { return mRight; }
        const glm::vec3 &eye()         const { return mEye;   }
        const glm::vec3 &center()      const { return mCenter;}
        const glm::mat4 viewMatrix()   const { return glm::lookAt(mEye, mCenter, mUp); }
        float zoom()                   const { return mZoom;  }
        void  rotateAroundCenter(const glm::quat &q);
        void  rotateAroundFront (const glm::quat &q);
        void  setQuatAroundCenter(const glm::quat &q);
        void  setQuatAroundFront (const glm::quat &q);
        const glm::quat &quatAroundFront() const { return mQuatAroundFront; }
        const glm::quat &quatAroundCenter() const { return mQuatAroundCenter; }

        static glm::quat QuatFromStandard(const glm::vec3 &up, const glm::vec3 &front);

        void reset() { this->_initialize(); }
    };

    /**
     * The camera controlled by arcball using mouse.
     **/
    class ArcballCamera : public CameraBase {
    public:
        // constructor
        ArcballCamera(glm::vec3 eye=glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 up=glm::vec3(0.0f, 1.0f, 0.0f),
                      glm::vec3 center=CameraBase::gOrigin);

        void  zoomIn(float deltaAngle=1.f)  { mZoom = std::max( 5.f, mZoom - std::max(0.f, deltaAngle * mSpeedZoom)); }
        void  zoomOut(float deltaAngle=1.f) { mZoom = std::min(85.f, mZoom + std::max(0.f, deltaAngle * mSpeedZoom)); }
        void  processMouseEvent(SDL_Event &event);

        void setSpeedZoom(float speed) { mSpeedZoom = speed; }
        void setSpeedMove(float speed) { mSpeedMove = speed; }
        void setSpeedRotate(float speed);

        Arcball *arcballPtr() { return mArcballPtr; }
        void reset();
    private:
        Arcball    *mArcballPtr;
        // for event
        int         mLastX, mLastY;
        int         mWidth, mHeight;
        glm::vec3   mLastPos;
        // speed
        float       mSpeedZoom, mSpeedMove, mSpeedRotate;
    };
}