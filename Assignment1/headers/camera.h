#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Target;
    glm::vec3 Up;

    Camera(
        glm::vec3 position = glm::vec3(0.0f, 2.0f, 5.0f),
        glm::vec3 target = glm::vec3(0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)
    )
        : Position(position), Target(target), Up(up) {}

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Target, Up);
    }
};

#endif
