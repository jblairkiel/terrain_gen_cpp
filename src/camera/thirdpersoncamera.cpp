#include "thirdpersoncamera.h"
#include <glm/gtc/matrix_transform.hpp>

static float clampPitch(float p)
{
    const float limit = glm::radians(89.0f);
    return glm::clamp(p, -limit, limit);
}

ThirdPersonCamera::ThirdPersonCamera(GLFWwindow *win,
                                     float dist,
                                     float h)
    : window(win),
      distance(dist),
      height(h),
      yaw(0.0f),
      pitch(0.0f),
      targetPos(0.0f),
      lastMouseX(0.0),
      lastMouseY(0.0),
      firstMouse(true),
      mode(CameraMode::Static)
{
}

void ThirdPersonCamera::setTargetPosition(const glm::vec3 &pos)
{
    targetPos = pos;
}

void ThirdPersonCamera::setMode(CameraMode m)
{
    mode = m;
}

glm::vec3 ThirdPersonCamera::computeOffset() const
{
    float x = std::sin(yaw) * std::cos(pitch) * distance;
    float z = std::cos(yaw) * std::cos(pitch) * distance;
    float y = std::sin(pitch) * distance + height;

    return glm::vec3(x, y, z);
}

void ThirdPersonCamera::handleMouseInput(float dt)
{
    if (!window || mode != CameraMode::Orbit)
        return;

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    if (firstMouse)
    {
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        firstMouse = false;
        return;
    }

    float dx = static_cast<float>(mouseX - lastMouseX);
    float dy = static_cast<float>(mouseY - lastMouseY);

    lastMouseX = mouseX;
    lastMouseY = mouseY;

    const float sensitivity = 0.0025f;
    yaw += dx * sensitivity;
    pitch -= dy * sensitivity;

    pitch = clampPitch(pitch);
}

void ThirdPersonCamera::update(float dt)
{
    handleMouseInput(dt);
    pitch = clampPitch(pitch);
}

glm::mat4 ThirdPersonCamera::getViewMatrix() const
{
    glm::vec3 offset = computeOffset();
    glm::vec3 camPos = targetPos + offset;

    return glm::lookAt(camPos, targetPos, glm::vec3(0, 1, 0));
}