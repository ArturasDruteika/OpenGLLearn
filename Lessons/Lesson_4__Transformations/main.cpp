#include "Utils/FileOperations/include/FileOperations.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "spdlog/spdlog.h"

#include <cmath>
#include <exception>
#include <filesystem>
#include <string>


constexpr int OPENGL_MAJOR_VERSION = 4;
constexpr int OPENGL_MINOR_VERSION = 6;
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float BACKGROUND_COLOR[4] = { 0.1f, 0.2f, 0.3f, 1.0f };


glm::mat3 CreateTranslation2D(const glm::vec2& translation)
{
    glm::mat3 result(1.0f);
    result[2] = glm::vec3(translation, 1.0f);
    return result;
}

glm::mat3 CreateRotation2D(float angleInRadians)
{
    const float cosine = std::cos(angleInRadians);
    const float sine = std::sin(angleInRadians);

    glm::mat3 result(1.0f);
    result[0] = glm::vec3(cosine, sine, 0.0f);
    result[1] = glm::vec3(-sine, cosine, 0.0f);
    return result;
}

glm::mat3 CreateScale2D(const glm::vec2& scale)
{
    glm::mat3 result(1.0f);
    result[0] = glm::vec3(scale.x, 0.0f, 0.0f);
    result[1] = glm::vec3(0.0f, scale.y, 0.0f);
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

    GLFWwindow* pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Triangle", nullptr, nullptr);
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
    
    float triangleVertices[] = {
        // positions    // colors
         0.0f,  0.25f,  0.0f, 1.0f, 1.0f, 1.0f,
        -0.25f, -0.25f, 0.0f, 1.0f, 1.0f, 1.0f,
         0.25f, -0.25f, 0.0f, 1.0f, 1.0f, 1.0f,
    };

    unsigned int vbo;
    unsigned int vao;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
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

    const float rotationAngle = glm::radians(90.0f);
    const glm::vec2 nonUniformScale{ 0.5f, 1.5f };
    const glm::vec2 topLeftPosition{ -0.5f, 0.5f };
    const glm::vec2 bottomLeftPosition{ -0.5f, -0.5f };
    const glm::vec2 topRightPosition{ 0.5f, 0.5f };
    const glm::vec2 bottomRightPosition{ 0.5f, -0.5f };

    const glm::mat3 translateTopLeft = CreateTranslation2D(topLeftPosition);
    const glm::mat3 translateBottomLeft = CreateTranslation2D(bottomLeftPosition);
    const glm::mat3 translateTopRight = CreateTranslation2D(topRightPosition);
    const glm::mat3 translateBottomRight = CreateTranslation2D(bottomRightPosition);

    const glm::mat3 rotate2D = CreateRotation2D(rotationAngle);
    const glm::mat3 scale2D = CreateScale2D(nonUniformScale);

    const glm::mat3 topLeft = translateTopLeft;
    const glm::mat3 bottomLeft = translateBottomLeft * rotate2D;
    const glm::mat3 topRight = translateTopRight * scale2D;
    const glm::mat3 bottomRight = translateBottomRight * rotate2D * scale2D;

    int transformLocation = glGetUniformLocation(shaderProgram, "u_transform");
    while (!glfwWindowShouldClose(pWindow))
    {
        glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], BACKGROUND_COLOR[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);

        glUniformMatrix3fv(transformLocation, 1, GL_FALSE, glm::value_ptr(topLeft));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniformMatrix3fv(transformLocation, 1, GL_FALSE, glm::value_ptr(bottomLeft));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniformMatrix3fv(transformLocation, 1, GL_FALSE, glm::value_ptr(topRight));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniformMatrix3fv(transformLocation, 1, GL_FALSE, glm::value_ptr(bottomRight));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    glfwDestroyWindow(pWindow);
    glfwTerminate();
}