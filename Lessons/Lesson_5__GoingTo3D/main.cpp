#include "Utils/FileOperations/include/FileOperations.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "spdlog/spdlog.h"

#include <cmath>
#include <exception>
#include <filesystem>
#include <string>


constexpr int OPENGL_MAJOR_VERSION = 4;
constexpr int OPENGL_MINOR_VERSION = 6;
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 800;
constexpr float BACKGROUND_COLOR[4] = { 0.1f, 0.2f, 0.3f, 1.0f };


glm::mat4 CreateRotationX3D(float angleInRadians)
{
    const float cosine = std::cos(angleInRadians);
    const float sine = std::sin(angleInRadians);

    glm::mat4 result(1.0f);
    result[1][1] = cosine;
    result[1][2] = sine;
    result[2][1] = -sine;
    result[2][2] = cosine;
    return result;
}

glm::mat4 CreateRotationY3D(float angleInRadians)
{
    const float cosine = std::cos(angleInRadians);
    const float sine = std::sin(angleInRadians);

    glm::mat4 result(1.0f);
    result[0][0] = cosine;
    result[0][2] = -sine;
    result[2][0] = sine;
    result[2][2] = cosine;
    return result;
}

glm::mat4 CreateRotationZ3D(float angleInRadians)
{
    const float cosine = std::cos(angleInRadians);
    const float sine = std::sin(angleInRadians);

    glm::mat4 result(1.0f);
    result[0][0] = cosine;
    result[0][1] = sine;
    result[1][0] = -sine;
    result[1][1] = cosine;
    return result;
}

glm::mat4 CreateScale3D(const glm::vec3& scale)
{
    glm::mat4 result(1.0f);
    result[0][0] = scale.x;
    result[1][1] = scale.y;
    result[2][2] = scale.z;
    return result;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int CompileShader(const std::string& source, unsigned int shaderID)
{
    const char* sourceCStr = source.c_str();
    glShaderSource(shaderID, 1, &sourceCStr, nullptr);
    glCompileShader(shaderID);

    int success;
    char infoLog[512];
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderID, 512, nullptr, infoLog);
        spdlog::error("Shader compilation failed: {}", infoLog);
    }

    return success;
}

unsigned int CreateShaderProgram(const std::string& vertexSource, const std::string& fragmentSource)
{
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!CompileShader(vertexSource, vertexShader))
    {
        glDeleteShader(vertexShader);
        return 0;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!CompileShader(fragmentSource, fragmentShader))
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        spdlog::error("Shader program linking failed: {}", infoLog);
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main()
{
    if (!glfwInit())
    {
        spdlog::error("Failed to initialize GLFW");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::string windowName = "3D Pyramid With EBO";

    GLFWwindow* pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowName.c_str(), nullptr, nullptr);
    if (!pWindow)
    {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(pWindow);
    glfwSetFramebufferSizeCallback(pWindow, framebufferSizeCallback);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        spdlog::error("Failed to initialize GLAD");
        glfwDestroyWindow(pWindow);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    float pyramidVertices[] =
    {
        // Front face vertices (Neon Blue 1)
         0.0f,  0.35f,  0.0f,   0.0f, 0.3f, 0.8f, 1.0f,   // 0
        -0.25f, -0.25f,  0.25f, 0.0f, 0.7f, 1.0f, 1.0f,   // 1
         0.25f, -0.25f,  0.25f, 0.0f, 0.7f, 1.0f, 1.0f,   // 2

        // Right face vertices (Neon Green)
         0.0f,  0.35f,  0.0f,   0.2f, 1.0f, 0.2f, 1.0f,   // 3
         0.25f, -0.25f,  0.25f, 0.2f, 1.0f, 0.2f, 1.0f,   // 4
         0.25f, -0.25f, -0.25f, 0.2f, 1.0f, 0.2f, 1.0f,   // 5

        // Back face vertices (Neon Purple)
         0.0f,  0.35f,  0.0f,   0.8f, 0.0f, 1.0f, 1.0f,   // 6
         0.25f, -0.25f, -0.25f, 0.8f, 0.0f, 1.0f, 1.0f,   // 7
        -0.25f, -0.25f, -0.25f, 0.8f, 0.0f, 1.0f, 1.0f,   // 8

        // Left face vertices (Neon Blue 2)
         0.0f,  0.35f,  0.0f,   0.0f, 0.4f, 0.8f, 1.0f,   // 9
        -0.25f, -0.25f, -0.25f, 0.0f, 0.8f, 1.0f, 1.0f,   // 10
        -0.25f, -0.25f,  0.25f, 0.0f, 0.8f, 1.0f, 1.0f,   // 11

        // Base vertices
        -0.25f, -0.25f,  0.25f, 0.0f, 0.4f, 0.8f, 1.0f,   // 12
        -0.25f, -0.25f, -0.25f, 0.0f, 0.4f, 0.8f, 1.0f,   // 13
         0.25f, -0.25f, -0.25f, 0.4f, 0.0f, 0.8f, 1.0f,   // 14
         0.25f, -0.25f,  0.25f, 0.4f, 0.0f, 0.8f, 1.0f    // 15
    };

    unsigned int pyramidIndices[] =
    {
        // Side faces
        0, 1, 2,       // Front
        3, 4, 5,       // Right
        6, 7, 8,       // Back
        9, 10, 11,     // Left

        // Base
        12, 13, 14,
        12, 14, 15
    };

    unsigned int vbo;
    unsigned int vao;
    unsigned int ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    std::string vertexShaderSource;
    std::string fragmentShaderSource;

    std::filesystem::path vertexShaderPath = "shaders/vertex.glsl";
    std::filesystem::path fragmentShaderPath = "shaders/fragment.glsl";

    try
    {
        vertexShaderSource = Orion::Utils::FileOperations::LoadFileAsString(vertexShaderPath);
        fragmentShaderSource = Orion::Utils::FileOperations::LoadFileAsString(fragmentShaderPath);
    }
    catch (const std::exception& exception)
    {
        spdlog::error("Failed to load shader files: {}", exception.what());
        glfwDestroyWindow(pWindow);
        glfwTerminate();
        return -1;
    }

    unsigned int shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (shaderProgram == 0)
    {
        glfwDestroyWindow(pWindow);
        glfwTerminate();
        return -1;
    }

    const float rotationAngleX = glm::radians(-20.0f);
    const float rotationAngleY = glm::radians(35.0f);
    const float rotationAngleZ = glm::radians(0.0f);
    const glm::vec3 scale{ 1.4f, 1.4f, 1.4f };

    const glm::mat4 rotateX3D = CreateRotationX3D(rotationAngleX);
    const glm::mat4 rotateY3D = CreateRotationY3D(rotationAngleY);
    const glm::mat4 rotateZ3D = CreateRotationZ3D(rotationAngleZ);
    const glm::mat4 scale3D = CreateScale3D(scale);

    const glm::mat4 model = rotateY3D * rotateX3D * rotateZ3D * scale3D;

    const glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 2.5f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    const glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
        0.1f,
        100.0f
    );

    int mvpLocation = glGetUniformLocation(shaderProgram, "u_mvp");
    if (mvpLocation == -1)
    {
        spdlog::error("Failed to find uniform location for u_mvp");
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glfwDestroyWindow(pWindow);
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(pWindow))
    {
        glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], BACKGROUND_COLOR[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);

        glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glfwDestroyWindow(pWindow);
    glfwTerminate();

    return 0;
}