#pragma once
#include <SDL2/SDL.h>
#include "snow_math.h"


/*
A unit quaternion q = cos(F)+u*sin(F) represents the rotation of vector v by the angle 2*F about axis u.

If your vectors are v and w, then we should normalize them, then calculate the angle between them as 2*F=ArcCos(Dot(v, w)). Rotation axis direction vector u = Normalize(VectorProduct(v, w)). Now we can build required rotation quaternion.

*/
namespace snow {

    class Arcball {
    private:
        float       mSpeed;
        float       mRadius;
        glm::vec3   mCenter;
        glm::vec3   mCurrPos;
        glm::vec3   mPrevPos;
    public:
    };
}