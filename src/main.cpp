#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include "camera/third_person/third_person.h"
#include "shader/shader.h"

// ----------------- Simple noise -----------------
float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
float lerp(float a, float b, float t) { return a + t * (b - a); }

static int p[512];
static int permutation[256] = {
    151, 160, 137, 91, 90, 15,
    131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
    190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
    88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
    77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
    102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
    135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
    5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
    223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
    251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
    49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
    138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180};

void initPerlin()
{
    for (int i = 0; i < 256; ++i)
    {
        p[256 + i] = p[i] = permutation[i];
    }
}

float grad(int hash, float x, float y)
{
    int h = hash & 3;
    float u = h < 2 ? x : y;
    float v = h < 2 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float perlin(float x, float y)
{
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;

    x -= floor(x);
    y -= floor(y);

    float u = fade(x);
    float v = fade(y);

    int aa = p[p[X] + Y];
    int ab = p[p[X] + Y + 1];
    int ba = p[p[X + 1] + Y];
    int bb = p[p[X + 1] + Y + 1];

    float res = lerp(
        lerp(grad(aa, x, y), grad(ba, x - 1, y), u),
        lerp(grad(ab, x, y - 1), grad(bb, x - 1, y - 1), u),
        v);
    return res;
}
// Shadow map size
static const unsigned int SHADOW_SIZE = 2048;

// ------------------------------------------------------------
// Globals from your existing terrain system
// ------------------------------------------------------------
extern GLuint terrainVAO;
extern std::vector<unsigned int> terrainIdx;

extern GLuint cubeVAO;
extern std::vector<unsigned int> cubeIdx;

extern float noiseScale;
extern float heightScale;

extern float sampleHeight(float x, float z, float noiseScale, float heightScale);

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main()
{
    // -----------------------------
    // GLFW + GLAD init
    // -----------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Terrain Engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    // -----------------------------
    // Load shaders
    // -----------------------------
    Shader program("shaders/main.vert", "shaders/main.frag");
    Shader shadowProgram("shaders/shadow.vert", "shaders/shadow.frag");

    // -----------------------------
    // Camera
    // -----------------------------
    ThirdPersonCamera camera(window, 20.0f, 5.0f);
    camera.setMode(CameraMode::Static);

    glm::vec3 playerPos(50.0f, 0.0f, 50.0f);

    // -----------------------------
    // Shadow map framebuffer
    // -----------------------------
    GLuint shadowFBO;
    glGenFramebuffers(1, &shadowFBO);

    GLuint shadowMap;
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_SIZE, SHADOW_SIZE, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = {1, 1, 1, 1};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // -----------------------------
    // Light setup
    // -----------------------------
    glm::vec3 lightDir = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.2f));
    glm::vec3 lightPos = -lightDir * 100.0f;

    glm::mat4 lightProjection = glm::ortho(-150.0f, 150.0f,
                                           -150.0f, 150.0f,
                                           1.0f, 300.0f);

    // -----------------------------
    // Main loop
    // -----------------------------
    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        // Update player height from terrain
        playerPos.y = sampleHeight(playerPos.x, playerPos.z, noiseScale, heightScale);

        // Update camera
        camera.setTargetPosition(playerPos);
        camera.update(dt);

        // Light view matrix
        glm::mat4 lightView = glm::lookAt(lightPos,
                                          glm::vec3(50, 0, 50),
                                          glm::vec3(0, 1, 0));

        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        // ------------------------------------------------------------
        // PASS 1: Shadow map
        // ------------------------------------------------------------
        glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        shadowProgram.use();
        shadowProgram.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // Terrain
        glm::mat4 model = glm::mat4(1.0f);
        shadowProgram.setMat4("uModel", model);

        glBindVertexArray(terrainVAO);
        glDrawElements(GL_TRIANGLES, terrainIdx.size(), GL_UNSIGNED_INT, 0);

        // Character cube
        glm::mat4 charModel = glm::translate(glm::mat4(1.0f), playerPos);
        shadowProgram.setMat4("uModel", charModel);

        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, cubeIdx.size(), GL_UNSIGNED_INT, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ------------------------------------------------------------
        // PASS 2: Main render
        // ------------------------------------------------------------
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program.use();

        glm::mat4 projection = glm::perspective(glm::radians(60.0f),
                                                (float)width / height,
                                                0.1f, 500.0f);

        glm::mat4 view = camera.getViewMatrix();

        program.setMat4("uProjection", projection);
        program.setMat4("uView", view);
        program.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // Bind shadow map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        program.setInt("shadowMap", 1);

        // Terrain
        program.setMat4("uModel", glm::mat4(1.0f));
        glBindVertexArray(terrainVAO);
        glDrawElements(GL_TRIANGLES, terrainIdx.size(), GL_UNSIGNED_INT, 0);

        // Character
        program.setMat4("uModel", charModel);
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, cubeIdx.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
