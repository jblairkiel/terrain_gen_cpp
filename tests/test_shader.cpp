#include <catch2/catch_test_macros.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../src/shaders/shader.h"
#include <fstream>

// Utility: create a temporary shader file
static std::string writeTempFile(const std::string &name, const std::string &contents)
{
    std::ofstream out(name);
    out << contents;
    out.close();
    return name;
}

// Utility: initialize a headless OpenGL context once
static bool initGL()
{
    static bool initialized = false;
    if (initialized)
        return true;

    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(1, 1, "", nullptr, nullptr);
    if (!window)
        return false;

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return false;

    initialized = true;
    return true;
}

TEST_CASE("Shader loads files correctly")
{
    REQUIRE(initGL());

    std::string vertPath = writeTempFile("temp_test.vert",
                                         "#version 330 core\nvoid main(){gl_Position=vec4(0.0);}");

    std::string fragPath = writeTempFile("temp_test.frag",
                                         "#version 330 core\nout vec4 c; void main(){c=vec4(1.0);}");

    REQUIRE_NOTHROW(Shader(vertPath, fragPath));
}

TEST_CASE("Shader reports compile errors")
{
    REQUIRE(initGL());

    std::string vertPath = writeTempFile("bad.vert",
                                         "#version 330 core\nERROR");

    std::string fragPath = writeTempFile("good.frag",
                                         "#version 330 core\nout vec4 c; void main(){c=vec4(1.0);}");

    // Shader constructor prints errors but should not crash
    REQUIRE_NOTHROW(Shader(vertPath, fragPath));
}

TEST_CASE("Shader reports link errors")
{
    REQUIRE(initGL());

    // Vertex shader outputs nothing, fragment shader expects input
    std::string vertPath = writeTempFile("link_fail.vert",
                                         "#version 330 core\nvoid main(){gl_Position=vec4(0.0);}");

    std::string fragPath = writeTempFile("link_fail.frag",
                                         "#version 330 core\nin vec3 missing; out vec4 c; void main(){c=vec4(missing,1.0);}");

    REQUIRE_NOTHROW(Shader(vertPath, fragPath));
}

TEST_CASE("Shader uniform setters do not crash")
{
    REQUIRE(initGL());

    std::string vertPath = writeTempFile("uniform.vert",
                                         "#version 330 core\nlayout(location=0) in vec3 aPos;\n"
                                         "uniform mat4 uModel;\n"
                                         "void main(){gl_Position=uModel*vec4(aPos,1.0);}");

    std::string fragPath = writeTempFile("uniform.frag",
                                         "#version 330 core\nout vec4 c; uniform float uValue; void main(){c=vec4(uValue);}");

    Shader shader(vertPath, fragPath);
    shader.use();

    REQUIRE_NOTHROW(shader.setFloat("uValue", 1.0f));
    REQUIRE_NOTHROW(shader.setInt("uValue", 1));
    REQUIRE_NOTHROW(shader.setBool("uValue", true));

    glm::mat4 m(1.0f);
    REQUIRE_NOTHROW(shader.setMat4("uModel", m));
}

TEST_CASE("Shader creates a valid GL program")
{
    REQUIRE(initGL());

    std::string vertPath = writeTempFile("valid.vert",
                                         "#version 330 core\nvoid main(){gl_Position=vec4(0.0);}");

    std::string fragPath = writeTempFile("valid.frag",
                                         "#version 330 core\nout vec4 c; void main(){c=vec4(1.0);}");

    Shader shader(vertPath, fragPath);

    REQUIRE(shader.ID != 0);
    REQUIRE(glIsProgram(shader.ID) == GL_TRUE);
}