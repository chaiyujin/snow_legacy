#pragma once

#include <glad/glad.h>
#include <core/math.h>

#include <vector>
#include <iostream>

namespace snow {
    class Camera {
    public:
        // constructor
        Camera(glm::vec3 position=glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3 up=glm::vec3(0.0f, 1.0f, 0.0f),
            glm::quat quaternion=glm::angleAxis(0.f, glm::vec3(0.f, 1.f, 0.f)))
            : mPosition(position)
            , mFront(glm::vec3(0.f, 0.f, -1.f))
            , mWorldUp(up)
            , mQuaternion(quaternion)
            , mZoom(45.f)
        {
            updateCameraVectors();
        }

        glm::mat4 getViewMatrix() {
            return glm::lookAt(mPosition, mPosition + mFront, mUp);
        }

        glm::vec3 up()          const { return mUp; }
        glm::vec3 front()       const { return mFront; }
        glm::vec3 position()    const { return mPosition; }
        glm::quat quaternion()  const { return mQuaternion; }
        float     zoom()        const { return mZoom; }

    private:
        // camera attributes
        glm::vec3   mPosition;
        glm::vec3   mFront;
        glm::vec3   mUp;
        glm::vec3   mRight;
        glm::vec3   mWorldUp;
        // quat
        glm::quat   mQuaternion;
        // options
        float       mZoom;

        void updateCameraVectors() {
            glm::vec3 front(0.f, 0.f, -1.f);
            mFront = glm::normalize(glm::rotate(mQuaternion, front));
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