#pragma once
#include <vector>
#include <SDL2/SDL.h>
#include "common.h"
#include "../core/snow_shader.h"

namespace snow {

    /**
     *  The arcball controlled by mouse.
     *  
     *    Pick p0 and move to p1 (both p0, p1 are on screen).
     *    We simply set the `z` of p0 and p1 as 2 * arcball radius.
     *    Then we rotate p0 and p1 according to current axises.
     *    Finally, we can calculate a quaterion from (p0 - O) to (p1 - O).
     * 
     *    We have to keep knowing which camera we are using to observe arcball,
     *    this tells us how to rotate p0 and p1 to correct position.
     * 
     **/

    class Arcball {
    private:
        const int       SampleNumber=100;
        std::vector<glm::vec3> mArcData;  // (x, y, z), (r, g, b)
        std::vector<uint32_t>  mArcIndices;
        Shader                 mShader;
        uint32_t               mVBO, mVAO, mEBO;

        CameraBase     *mCamera;
        bool            mIsCamera;
        int             mHalfHeight, mHalfWidth;
        float           mRadiusOfHalfHeight;
        glm::vec3       mCenter;
        glm::vec2       mCurrPos;
        glm::vec2       mPrevPos;
        glm::quat       mQuatCamera;  // the camera rotation, which affect arcball coordinate
        glm::quat       mQuatObject;  // the object rotation
        glm::quat       mDelta;
        bool            mIsMoving;
        float           mSpeed;

        void            _updateDelta();
        void            _generateArc();

        glm::mat4       _modelMatrix();

    public:
        Arcball(CameraBase *camera,
                bool manipulateCamera, 
                float radiusOfHalfHeight=0.5f,
                glm::vec3 center=glm::vec3(0.f,0.f,0.f));
        glm::quat quaternion();
        void processMouseEvent(SDL_Event &event);
        void setSpeed(float speed) { mSpeed = speed; }

        void draw(const glm::mat4 &project);
    };
}