#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm.hpp>
#include <gtx/transform.hpp>

namespace gps {

    enum MOVE_DIRECTION {
        MOVE_FORWARD,
        MOVE_BACKWARD,
        MOVE_RIGHT,
        MOVE_LEFT
    };

    class Camera
    {
    public:
        // Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);

        // Returns the view matrix, using the glm::lookAt() function
        glm::mat4 getViewMatrix();

        // Moves the camera in the specified direction with a certain speed
        void move(MOVE_DIRECTION direction, float speed);

        // Rotates the camera by pitchOffset and yawOffset
        //   pitch: rotation around the X axis
        //   yaw:   rotation around the Y axis
        void rotate(float pitchOffset, float yawOffset);

        glm::vec3 getPosition() const;

    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
    };

}

#endif /* CAMERA_HPP */