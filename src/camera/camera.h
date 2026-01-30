#pragma once
#include <glm/glm.hpp>

class Camera
{
public:
    virtual ~Camera() = default;

    // Update camera state (input, smoothing, etc.)
    virtual void update(float dt) = 0;

    // Return the view matrix
    virtual glm::mat4 getViewMatrix() const = 0;

    // Set the position the camera should follow or look at
    virtual void setTargetPosition(const glm::vec3 &pos) = 0;
};