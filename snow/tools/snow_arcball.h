#pragma once
#include <SDL2/SDL.h>
#include "common.h"

/*
A unit quaternion q = cos(F)+u*sin(F) 
represents the rotation of vector v by the angle 2*F about axis u.

If your vectors are v and w, then we should normalize them,
then calculate the angle between them as 2*F=ArcCos(Dot(v, w)). 
Rotation axis direction vector u = Normalize(VectorProduct(v, w)). 
Now we can build required rotation quaternion.
*/
namespace snow {

    class Arcball {
    private:
        CameraBase *mCamera;
        bool            mIsCamera;
        int             mHalfHeight, mHalfWidth;
        float           mRadiusOfHalfHeight;
        glm::vec3       mCenter;
        glm::vec2       mCurrPos;
        glm::vec2       mPrevPos;
        glm::quat       mQuatCam;  // the camera rotation, which affect arcball coordinate
        glm::quat       mQuat;  // the object rotation
        glm::quat       mDelta;
        bool            mIsMoving;

        void        _updateDelta();
    public:
        Arcball(CameraBase *camera, bool manipulateCamera, 
                float radiusOfHalfHeight=0.5f, glm::vec3 center=glm::vec3(0.f,0.f,0.f));

        glm::quat quaternion();
        void processMouseEvent(SDL_Event &event);
    };
    
}