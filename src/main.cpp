#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include "camera/thirdpersoncamera.h"

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

// ----------------- Shader helpers -----------------
GLuint compileShader(GLenum type, const char *src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint success;
    glGetShaderiv(s, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
    }
    return s;
}

GLuint createProgram(const char *vsSrc, const char *fsSrc)
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log);
        std::cerr << "Program link error: " << log << std::endl;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// Put your shader source here as raw strings for simplicity
const char *terrainVertSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
out vec3 vNormal;
out vec3 vWorldPos;
void main()
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    gl_Position = uProjection * uView * worldPos;
}
)";

const char *terrainFragSrc = R"(
#version 330 core
in vec3 vNormal;
in vec3 vWorldPos;
out vec4 FragColor;
uniform vec3 uLightDir = normalize(vec3(0.3, 1.0, 0.2));
uniform vec3 uColor;
void main()
{
    float diff = max(dot(normalize(vNormal), -uLightDir), 0.1);
    vec3 base = uColor;
    vec3 color = base * diff;
    FragColor = vec4(color, 1.0);
}
)";

// ----------------- Terrain mesh -----------------
struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
};

void computeNormals(std::vector<Vertex> &verts, const std::vector<unsigned int> &indices)
{
    for (auto &v : verts)
        v.normal = glm::vec3(0.0f);
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        Vertex &v0 = verts[indices[i]];
        Vertex &v1 = verts[indices[i + 1]];
        Vertex &v2 = verts[indices[i + 2]];
        glm::vec3 e1 = v1.pos - v0.pos;
        glm::vec3 e2 = v2.pos - v0.pos;
        glm::vec3 n = glm::normalize(glm::cross(e1, e2));
        v0.normal += n;
        v1.normal += n;
        v2.normal += n;
    }
    for (auto &v : verts)
        v.normal = glm::normalize(v.normal);
}

void generateTerrain(int width, int depth, float scale, float heightScale,
                     std::vector<Vertex> &vertices,
                     std::vector<unsigned int> &indices)
{
    vertices.clear();
    indices.clear();
    vertices.reserve(width * depth);

    for (int z = 0; z < depth; ++z)
    {
        for (int x = 0; x < width; ++x)
        {
            float nx = x * scale;
            float nz = z * scale;
            float h = perlin(nx, nz) * heightScale;
            Vertex v;
            v.pos = glm::vec3((float)x, h, (float)z);
            v.normal = glm::vec3(0.0f);
            vertices.push_back(v);
        }
    }

    for (int z = 0; z < depth - 1; ++z)
    {
        for (int x = 0; x < width - 1; ++x)
        {
            int i0 = z * width + x;
            int i1 = z * width + x + 1;
            int i2 = (z + 1) * width + x;
            int i3 = (z + 1) * width + x + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    computeNormals(vertices, indices);
}

// ----------------- Character (simple cube) -----------------
void createCube(std::vector<Vertex> &verts, std::vector<unsigned int> &idx)
{
    verts = {
        {{-0.5f, 0.0f, -0.5f}, {}},
        {{0.5f, 0.0f, -0.5f}, {}},
        {{0.5f, 1.0f, -0.5f}, {}},
        {{-0.5f, 1.0f, -0.5f}, {}},
        {{-0.5f, 0.0f, 0.5f}, {}},
        {{0.5f, 0.0f, 0.5f}, {}},
        {{0.5f, 1.0f, 0.5f}, {}},
        {{-0.5f, 1.0f, 0.5f}, {}},
    };

    idx = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 4, 7, 7, 3, 0,
        1, 5, 6, 6, 2, 1,
        3, 2, 6, 6, 7, 3,
        0, 1, 5, 5, 4, 0};

    computeNormals(verts, idx);
}

// ----------------- Input -----------------
float characterX = 10.0f;
float characterZ = 10.0f;
float characterSpeed = 5.0f;

void processInput(GLFWwindow *window, ThirdPersonCamera &camera, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        characterZ -= characterSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        characterZ += characterSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        characterX -= characterSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        characterX += characterSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camera.setMode(CameraMode::Static);
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        camera.setMode(CameraMode::Orbit);
}

// Sample terrain height at (x,z) using same noise
float sampleHeight(float x, float z, float scale, float heightScale)
{
    return perlin(x * scale, z * scale) * heightScale;
}

// ----------------- Main -----------------
int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1920, 1080, "Procedural Terrain with Character", nullptr, nullptr);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);

    initPerlin();

    GLuint program = createProgram(terrainVertSrc, terrainFragSrc);

    // Generate terrain
    std::vector<Vertex> terrainVerts;
    std::vector<unsigned int> terrainIdx;
    int terrainW = 128;
    int terrainD = 128;
    float noiseScale = 0.05f;
    float heightScale = 5.0f;
    generateTerrain(terrainW, terrainD, noiseScale, heightScale, terrainVerts, terrainIdx);

    GLuint terrainVAO, terrainVBO, terrainEBO;
    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glGenBuffers(1, &terrainEBO);

    glBindVertexArray(terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, terrainVerts.size() * sizeof(Vertex), terrainVerts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrainIdx.size() * sizeof(unsigned int), terrainIdx.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Character cube
    std::vector<Vertex> cubeVerts;
    std::vector<unsigned int> cubeIdx;
    createCube(cubeVerts, cubeIdx);

    GLuint cubeVAO, cubeVBO, cubeEBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVerts.size() * sizeof(Vertex), cubeVerts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIdx.size() * sizeof(unsigned int), cubeIdx.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Camera
    // glm::vec3 camPos(64.0f, 30.0f, 64.0f);
    ThirdPersonCamera camera(window, 20.0f, 10.0f);
    glm::vec3 camPos(50.0f, 50.0f, 50.0f);
    // glm::vec3 camTarget(64.0f, 0.0f, 64.0f);
    glm::vec3 camTarget(0.0f, 0.0f, 0.0f);
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1280.0f / 720.0f, 0.1f, 500.0f);

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        float dt = (float)(currentTime - lastTime);
        lastTime = currentTime;

        glfwPollEvents();
        processInput(window, camera, dt);

        // Keep character within terrain bounds
        characterX = glm::clamp(characterX, 0.0f, (float)(terrainW - 1));
        characterZ = glm::clamp(characterZ, 0.0f, (float)(terrainD - 1));

        float charY = sampleHeight(characterX, characterZ, noiseScale, heightScale);

        glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);

        // Update camera target from character
        glm::vec3 charPos(characterX, charY, characterZ);
        camera.setTargetPosition(charPos);
        camera.update(dt);

        glm::mat4 view = camera.getViewMatrix();

        GLint locModel = glGetUniformLocation(program, "uModel");
        GLint locView = glGetUniformLocation(program, "uView");
        GLint locProj = glGetUniformLocation(program, "uProjection");
        GLint locColor = glGetUniformLocation(program, "uColor");

        glUniformMatrix4fv(locView, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(locProj, 1, GL_FALSE, &projection[0][0]);

        // Draw terrain
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(locModel, 1, GL_FALSE, &model[0][0]);
        glUniform3f(locColor, 0.2f, 0.7f, 0.3f);

        glBindVertexArray(terrainVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)terrainIdx.size(), GL_UNSIGNED_INT, 0);

        // Draw character
        glm::mat4 charModel = glm::translate(glm::mat4(1.0f), glm::vec3(characterX, charY, characterZ));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, &charModel[0][0]);
        glUniform3f(locColor, 0.8f, 0.2f, 0.2f);

        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)cubeIdx.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}