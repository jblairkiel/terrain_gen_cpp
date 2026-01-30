#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../src/camera/thirdpersoncamera.h"

// A dummy GLFWwindow pointer for construction.
// We never call GLFW functions in these tests.
static GLFWwindow *dummyWindow = nullptr;

TEST_CASE("ThirdPersonCamera computes correct offset for yaw=0 pitch=0")
{
    ThirdPersonCamera cam(dummyWindow, 10.0f, 5.0f);
    cam.setTargetPosition(glm::vec3(0.0f));

    cam.yaw = 0.0f;
    cam.pitch = 0.0f;

    glm::mat4 view = cam.getViewMatrix();

    // Camera should be at (0,5,10) looking at (0,0,0)
    glm::vec3 expectedPos(0.0f, 5.0f, 10.0f);

    glm::vec3 actualPos = glm::inverse(view)[3];

    REQUIRE(actualPos.x == Catch::Approx(expectedPos.x));
    REQUIRE(actualPos.y == Catch::Approx(expectedPos.y));
    REQUIRE(actualPos.z == Catch::Approx(expectedPos.z));
}

TEST_CASE("ThirdPersonCamera offset rotates correctly with yaw")
{
    ThirdPersonCamera cam(dummyWindow, 10.0f, 5.0f);
    cam.setTargetPosition(glm::vec3(0.0f));

    cam.yaw = glm::radians(90.0f);
    cam.pitch = 0.0f;

    glm::mat4 view = cam.getViewMatrix();
    glm::mat4 inv = glm::inverse(view);
    glm::vec3 camPos(inv[3][0], inv[3][1], inv[3][2]);
    camPos /= inv[3][3];
    /// glm::vec3 camPos(inv[3][0], inv[3][1], inv[3][2]);

    // yaw=90° → camera should move to +X direction
    REQUIRE(camPos.x == Catch::Approx(10.0f).margin(0.001f));
    REQUIRE(camPos.z == Catch::Approx(0.0f).margin(0.001f));
    REQUIRE(camPos.y == Catch::Approx(5.0f).margin(0.001f));
}

TEST_CASE("ThirdPersonCamera pitch affects height")
{
    ThirdPersonCamera cam(dummyWindow, 10.0f, 0.0f);
    cam.setTargetPosition(glm::vec3(0.0f));

    cam.yaw = 0.0f;
    cam.pitch = glm::radians(45.0f);

    glm::mat4 view = cam.getViewMatrix();
    glm::vec3 camPos = glm::inverse(view)[3];

    // sin(45°) * 10 = 7.071
    REQUIRE(camPos.y == Catch::Approx(7.071f).epsilon(0.01f));
}

TEST_CASE("ThirdPersonCamera clamps pitch to safe range")
{
    ThirdPersonCamera cam(dummyWindow, 10.0f, 0.0f);
    cam.setTargetPosition(glm::vec3(0.0f));

    cam.pitch = glm::radians(200.0f); // absurd
    cam.update(0.016f);

    REQUIRE(cam.pitch <= glm::radians(89.0f));
    REQUIRE(cam.pitch >= glm::radians(-89.0f));
}

TEST_CASE("ThirdPersonCamera view matrix looks at target")
{
    ThirdPersonCamera cam(dummyWindow, 10.0f, 5.0f);

    glm::vec3 target(50.0f, 0.0f, 50.0f);
    cam.setTargetPosition(target);

    cam.yaw = glm::radians(180.0f);
    cam.pitch = 0.0f;

    glm::mat4 view = cam.getViewMatrix();
    glm::vec3 camPos = glm::inverse(view)[3];

    // Camera should be behind the target on -Z
    REQUIRE(camPos.z < target.z);
}