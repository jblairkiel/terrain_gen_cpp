#pragma once
#include "camera.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

enum class CameraMode
{
    Static,
    Orbit
};

class ThirdPersonCamera : public Camera
{
public:
    ThirdPersonCamera(GLFWwindow *window,
                      float distance = 10.0f,
                      float height = 5.0f);

    void update(float dt) override;
    glm::mat4 getViewMatrix() const override;
    void setTargetPosition(const glm::vec3 &pos) override;

    void setMode(CameraMode mode);

    float yaw;
    float pitch;

private:
    GLFWwindow *window;

    glm::vec3 targetPos;
    float distance;
    float height;

    double lastMouseX;
    double lastMouseY;
    bool firstMouse;

    CameraMode mode;

    void handleMouseInput(float dt);
    glm::vec3 computeOffset() const;
};