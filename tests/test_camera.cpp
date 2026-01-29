#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "../src/camera/third_person/third_person.h"

TEST_CASE("Offset when pitch = 0")
{
    glm::vec3 offset = computeCameraOffset(0.0f, 0.0f, 10.0f, 5.0f);

    REQUIRE(offset.x == Catch::Approx(0.0f));
    REQUIRE(offset.z == Catch::Approx(10.0f));
    REQUIRE(offset.y == Catch::Approx(5.0f));
}

TEST_CASE("Offset when looking upward")
{
    glm::vec3 offset = computeCameraOffset(0.0f, 0.5f, 10.0f, 5.0f);

    REQUIRE(offset.y > 5.0f);
    REQUIRE(offset.z < 10.0f);
}

TEST_CASE("Offset when looking downward")
{
    glm::vec3 offset = computeCameraOffset(0.0f, -0.5f, 10.0f, 5.0f);

    REQUIRE(offset.y < 5.0f);
    REQUIRE(offset.z < 10.0f);
}

TEST_CASE("Yaw rotates camera around target")
{
    float pitch = 0.0f;
    float distance = 10.0f;
    float height = 5.0f;

    glm::vec3 offset0 = computeCameraOffset(0.0f, pitch, distance, height);
    glm::vec3 offset90 = computeCameraOffset(glm::half_pi<float>(), pitch, distance, height);

    REQUIRE(offset0.z == Catch::Approx(10.0f));
    REQUIRE(offset90.x == Catch::Approx(10.0f));
}