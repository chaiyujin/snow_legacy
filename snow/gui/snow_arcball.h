#pragma once
#include <SDL2/SDL.h>
#include "../core/snow_math.h"


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
        int         mHalfHeight, mHalfWidth;
        float       mRadiusOfHalfHeight;
        glm::vec3   mCenter;
        glm::vec2   mCurrPos;
        glm::vec2   mPrevPos;
        glm::quat   mQuat;
        glm::quat   mDelta;
        bool        mIsMoving;

        void        _updateDelta();
    public:
        Arcball(float radiusOfHalfHeight=0.3f, glm::vec3 center=glm::vec3(0.f,0.f,0.f));

        glm::quat quaternion() { return (mIsMoving)? glm::cross(mQuat, mDelta) : mQuat; }
        void processMouseEvent(SDL_Event &event);
    };
}