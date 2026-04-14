#include "Utils/FileOperations/include/FileOperations.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "spdlog/spdlog.h"

#include <cmath>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>


constexpr int OPENGL_MAJOR_VERSION = 4;
constexpr int OPENGL_MINOR_VERSION = 6;
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 800;
constexpr float BACKGROUND_COLOR[4] = { 0.1f, 0.2f, 0.3f, 1.0f };


struct Vertex
{
    glm::vec3 position;
    glm::vec4 color;
};


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

    const glm::vec3 top = { 0.0f, 0.35f, 0.0f };
    const glm::vec3 frontLeft = { -0.25f, -0.25f, 0.25f };
    const glm::vec3 frontRight = { 0.25f, -0.25f, 0.25f };
    const glm::vec3 backLeft = { -0.25f, -0.25f, -0.25f };
    const glm::vec3 backRight = { 0.25f, -0.25f, -0.25f };

    const glm::vec4 neonBlue1 = { 0.0f, 0.3f, 0.8f, 1.0f };
    const glm::vec4 neonBlue2 = { 0.0f, 0.7f, 1.0f, 1.0f };
    const glm::vec4 neonGreen = { 0.2f, 1.0f, 0.2f, 1.0f };
    const glm::vec4 neonPurple = { 0.8f, 0.0f, 1.0f, 1.0f };
    const glm::vec4 baseBlue = { 0.0f, 0.4f, 0.8f, 1.0f };
    const glm::vec4 basePurple = { 0.4f, 0.0f, 0.8f, 1.0f };

    const std::vector<Vertex> vertices =
    {
        // Front face
        { top, neonBlue1 },       // 0
        { frontLeft, neonBlue2 }, // 1
        { frontRight, neonBlue2 },// 2

        // Right face
        { top, neonGreen },       // 3
        { frontRight, neonGreen },// 4
        { backRight, neonGreen }, // 5

        // Back face
        { top, neonPurple },      // 6
        { backRight, neonPurple },// 7
        { backLeft, neonPurple }, // 8

        // Left face
        { top, neonBlue2 },       // 9
        { backLeft, neonBlue1 },  // 10
        { frontLeft, neonBlue1 }, // 11

        // Base
        { frontLeft, baseBlue },   // 12
        { backLeft, baseBlue },    // 13
        { backRight, basePurple }, // 14
        { frontRight, basePurple } // 15
    };

    const std::vector<unsigned int> indices =
    {
        // Side faces
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,

        // Base
        12, 13, 14,
        12, 14, 15
    };

    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
        indices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, position))
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, color))
    );
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
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glfwDestroyWindow(pWindow);
        glfwTerminate();
        return -1;
    }

    unsigned int shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (shaderProgram == 0)
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glfwDestroyWindow(pWindow);
        glfwTerminate();
        return -1;
    }

    const float rotationAngleX = glm::radians(-20.0f);
    const float rotationAngleY = glm::radians(35.0f);
    const float rotationAngleZ = glm::radians(0.0f);
    const glm::vec3 scale = { 1.4f, 1.4f, 1.4f };

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

        const glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(indices.size()),
            GL_UNSIGNED_INT,
            nullptr
        );

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