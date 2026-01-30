// ThirdPersonCamera.h
#pragma once
#include "../camera.h"
#include <GLFW/glfw3.h>

inline glm::vec3 computeCameraOffset(float yaw, float pitch, float distance, float height)
{
    glm::vec3 offset;
    offset.x = sin(yaw) * cos(pitch) * distance;
    offset.z = cos(yaw) * cos(pitch) * distance;
    offset.y = sin(pitch) * distance + height;
    return offset;
}

enum class CameraMode
{
    Static,
    Orbit
};
CameraMode mode = CameraMode::Static;
class ThirdPersonCamera : public Camera
{
public:
    ThirdPersonCamera(GLFWwindow *window, float distance, float height)
        : window(window), distance(distance), height(height)
    {
        // Initialize last mouse position
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        lastMouseX = x;
        lastMouseY = y;

        // Capture cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void setTargetPosition(const glm::vec3 &pos)
    {
        target = pos;
    }

    void update(float dt)
    {
        if (mode == CameraMode::Orbit)
        {
            handleMouseInput();
        }

        float yawToUse = (mode == CameraMode::Static) ? staticYaw : yaw;
        float pitchToUse = (mode == CameraMode::Static) ? staticPitch : pitch;

        glm::vec3 offset = computeCameraOffset(
            yawToUse,
            pitchToUse,
            distance,
            height);

        position = target + offset;
    }
    void setMode(CameraMode newMode)
    {
        mode = newMode;
    }

    glm::mat4 getViewMatrix() const override
    {
        return glm::lookAt(position, target, glm::vec3(0, 1, 0));
    }

private:
    GLFWwindow *window;
    float distance;
    float height;

    float yaw = 0.0f;
    float pitch = 0.0f;
    float staticYaw = glm::radians(180.0f);  // behind the player
    float staticPitch = glm::radians(30.0f); // slight downward angle

    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    float mouseSensitivity = 0.002f;
    bool useAbsoluteMouse = true; // toggle between modes
    float deadZone = 0.1f;        // 10% of screen

    void handleMouseInput()
    {
        if (!useAbsoluteMouse)
            return; // skip if using relative mode

        int winW, winH;
        glfwGetWindowSize(window, &winW, &winH);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        double centerX = winW * 0.5;
        double centerY = winH * 0.5;

        // How far is the cursor from the center?
        double offsetX = mouseX - centerX;
        double offsetY = mouseY - centerY;

        // Normalize to [-1, 1]
        float nx = static_cast<float>(offsetX / centerX);
        float ny = static_cast<float>(offsetY / centerY);

        if (fabs(nx) < deadZone)
            nx = 0.0f;
        if (fabs(ny) < deadZone)
            ny = 0.0f;

        nx = nx * nx * (nx > 0 ? 1 : -1);
        ny = ny * ny * (ny > 0 ? 1 : -1);

        // Apply sensitivity
        yaw -= nx * mouseSensitivity;
        pitch -= ny * mouseSensitivity;

        // Clamp pitch
        pitch = glm::clamp(pitch, -1.2f, 1.2f);
    }
};