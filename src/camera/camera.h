// Camera.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    virtual ~Camera() = default;

    virtual void update(float dt) = 0;
    virtual glm::mat4 getViewMatrix() const = 0;

protected:
    glm::vec3 position{0.0f};
    glm::vec3 target{0.0f};
};