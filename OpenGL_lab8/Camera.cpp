#include "Camera.hpp"
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp> // optional

namespace gps {

    // Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp)
    {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        // Calculate initial front and right directions
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, cameraUpDirection));

    }

    // Return the view matrix, using glm::lookAt
    glm::mat4 Camera::getViewMatrix()
    {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    // Move the camera using the WASD-like direction
    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
        switch (direction)
        {
        case MOVE_LEFT:
            cameraPosition -= speed * cameraRightDirection;
            cameraTarget -= speed * cameraRightDirection;
            break;

        case MOVE_RIGHT:
            cameraPosition += speed * cameraRightDirection;
            cameraTarget += speed * cameraRightDirection;
            break;

        case MOVE_FORWARD:
            cameraPosition += speed * cameraFrontDirection;
            cameraTarget += speed * cameraFrontDirection;
            break;

        case MOVE_BACKWARD:
            cameraPosition -= speed * cameraFrontDirection;
            cameraTarget -= speed * cameraFrontDirection;
            break;
        }

        // Recalculate front/right after moving
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }

    // Rotate camera by pitch and yaw offsets
    void Camera::rotate(float pitchOffset, float yawOffset)
    {
        glm::vec3 front;
        front.x = cos(glm::radians(yawOffset)) * cos(glm::radians(pitchOffset));
        front.y = sin(glm::radians(pitchOffset));
        front.z = sin(glm::radians(yawOffset)) * cos(glm::radians(pitchOffset));

        cameraFrontDirection = glm::normalize(front);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));

        cameraTarget = cameraPosition + cameraFrontDirection;
    }

    glm::vec3 Camera::getPosition() const
    {
        return cameraPosition;
    }

}
