#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Simple Perlin Noise implementation (based on public domain code)
class PerlinNoise
{
private:
    std::vector<int> p;

public:
    PerlinNoise(unsigned int seed = 0)
    {
        p.resize(512);
        for (int i = 0; i < 256; ++i)
            p[i] = i;
        std::srand(seed);
        for (int i = 255; i > 0; --i)
        {
            int j = std::rand() % (i + 1);
            std::swap(p[i], p[j]);
        }
        for (int i = 0; i < 256; ++i)
            p[256 + i] = p[i];
    }

    double noise(double x, double y, double z)
    {
        int X = (int)std::floor(x) & 255;
        int Y = (int)std::floor(y) & 255;
        int Z = (int)std::floor(z) & 255;

        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);

        double u = fade(x);
        double v = fade(y);
        double w = fade(z);

        int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
        int B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;

        return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)), lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))),
                    lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1)),
                         lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                              grad(p[BB + 1], x - 1, y - 1, z - 1))));
    }

private:
    double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    double lerp(double t, double a, double b) { return a + t * (b - a); }
    double grad(int hash, double x, double y, double z)
    {
        int h = hash & 15;
        double u = h < 8 ? x : y;
        double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
};

// Vertex shader source
const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec3 aNormal;\n"
                                 "uniform mat4 model;\n"
                                 "uniform mat4 view;\n"
                                 "uniform mat4 projection;\n"
                                 "out vec3 Normal;\n"
                                 "out vec3 FragPos;\n"
                                 "void main() {\n"
                                 "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
                                 "    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
                                 "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
                                 "}\0";

// Fragment shader source
const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec3 Normal;\n"
                                   "in vec3 FragPos;\n"
                                   "uniform vec3 lightPos;\n"
                                   "uniform vec3 viewPos;\n"
                                   "uniform vec3 lightColor;\n"
                                   "uniform vec3 objectColor;\n"
                                   "void main() {\n"
                                   "    float ambientStrength = 0.1;\n"
                                   "    vec3 ambient = ambientStrength * lightColor;\n"
                                   "    vec3 norm = normalize(Normal);\n"
                                   "    vec3 lightDir = normalize(lightPos - FragPos);\n"
                                   "    float diff = max(dot(norm, lightDir), 0.0);\n"
                                   "    vec3 diffuse = diff * lightColor;\n"
                                   "    float specularStrength = 0.5;\n"
                                   "    vec3 viewDir = normalize(viewPos - FragPos);\n"
                                   "    vec3 reflectDir = reflect(-lightDir, norm);\n"
                                   "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
                                   "    vec3 specular = specularStrength * spec * lightColor;\n"
                                   "    vec3 result = (ambient + diffuse + specular) * objectColor;\n"
                                   "    FragColor = vec4(result, 1.0);\n"
                                   "}\0";
bool mouseCaptured = true;

// Camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 20.0f, 80.0f);
// glm::vec3 cameraPos = glm::vec3(0.0f, 20.0f, 80.0f)

glm::vec3 cameraFront = glm::vec3(0.0f, -0.2f, -1.0f); // looking slightly down at start
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;   // looking along -Z initially
float pitch = -10.0f; // slightly looking down

float lastX = 400, lastY = 300; // center of 800×600 window
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float movementSpeed = 25.0f;    // units per second
float mouseSensitivity = 0.15f; // degrees per pixel

// Mouse movement callback
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    // Calculate mouse delta
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // inverted: down is negative
    lastX = xpos;
    lastY = ypos;

    // Apply sensitivity
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    // Update angles
    yaw += xoffset;
    pitch += yoffset;

    // Clamp pitch to prevent flipping
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Calculate camera direction from yaw and pitch
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// Mouse enter/leave to handle cursor capture
void cursor_enter_callback(GLFWwindow *window, int entered)
{
    if (entered)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true; // reset on re-enter
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

// Function to compile shader
unsigned int compileShader(const char *source, GLenum type)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    // Check for errors (omitted for brevity)
    return shader;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int old_main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Procedural Terrain", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetCursorEnterCallback(window, cursor_enter_callback);

    // Compile shaders
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check for errors (omitted)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Terrain parameters
    const int terrainSize = 100;
    const float scale = 0.05f;
    const float amplitude = 10.0f;
    PerlinNoise perlin(237); // Seed

    // Generate vertices and indices
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int z = 0; z < terrainSize; ++z)
    {
        for (int x = 0; x < terrainSize; ++x)
        {
            float y = amplitude * perlin.noise(x * scale, 0, z * scale);
            vertices.push_back(static_cast<float>(x) - terrainSize / 2.0f);
            vertices.push_back(y);
            vertices.push_back(static_cast<float>(z) - terrainSize / 2.0f);

            // Placeholder normals (will compute later)
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }
    }

    // Generate indices for triangle strips
    for (int z = 0; z < terrainSize - 1; ++z)
    {
        for (int x = 0; x < terrainSize; ++x)
        {
            indices.push_back(z * terrainSize + x);
            indices.push_back((z + 1) * terrainSize + x);
        }
        // Degenerate triangles for strip continuity
        if (z < terrainSize - 2)
        {
            indices.push_back((z + 1) * terrainSize + (terrainSize - 1));
            indices.push_back((z + 2) * terrainSize);
        }
    }

    // Compute normals
    for (size_t i = 0; i < indices.size() - 2; i += 3)
    {
        int i0 = indices[i] * 6;
        int i1 = indices[i + 1] * 6;
        int i2 = indices[i + 2] * 6;

        glm::vec3 v0(vertices[i0], vertices[i0 + 1], vertices[i0 + 2]);
        glm::vec3 v1(vertices[i1], vertices[i1 + 1], vertices[i1 + 2]);
        glm::vec3 v2(vertices[i2], vertices[i2 + 1], vertices[i2 + 2]);

        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        // Add to each vertex (simple accumulation)
        vertices[i0 + 3] += normal.x;
        vertices[i0 + 4] += normal.y;
        vertices[i0 + 5] += normal.z;
        vertices[i1 + 3] += normal.x;
        vertices[i1 + 4] += normal.y;
        vertices[i1 + 5] += normal.z;
        vertices[i2 + 3] += normal.x;
        vertices[i2 + 4] += normal.y;
        vertices[i2 + 5] += normal.z;
    }

    // Normalize normals
    for (size_t i = 0; i < vertices.size(); i += 6)
    {
        glm::vec3 normal(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
        normal = glm::normalize(normal);
        vertices[i + 3] = normal.x;
        vertices[i + 4] = normal.y;
        vertices[i + 5] = normal.z;
    }

    // Setup VAO, VBO, EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process movement
        float cameraSpeed = movementSpeed * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraUp;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraUp;

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        {
            mouseCaptured = !mouseCaptured;
            glfwSetInputMode(window, GLFW_CURSOR, mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            firstMouse = true;
            glfwSetCursorPos(window, 400, 300); // recenter
        }

        // Also keep ESC working
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Rendering
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Set uniforms
        glm::mat4 model = glm::mat4(1.0f);

        /// static
        // glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 50.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // dynamic
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 50.0f, 100.0f, 50.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 0.0f, 50.0f, 100.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.5f, 0.8f, 0.3f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}