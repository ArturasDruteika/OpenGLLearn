#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "Math/Constants/include/Constants.hpp"
#include "Math/LinearAlgebra/include/LinearAlgebraDataTypes.hpp"
#include "Math/LinearAlgebra/include/LinearAlgebraOperations.hpp"
#include "Math/Trigonometry/include/Trigonometry.hpp"
#include "spdlog/spdlog.h"
#include <cmath>
#include <string>


constexpr int OPENGL_MAJOR_VERSION = 4;
constexpr int OPENGL_MINOR_VERSION = 6;
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float BACKGROUND_COLOR[4] = { 0.1f, 0.2f, 0.3f, 1.0f };


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

const float* GetMatrixData(const Orion::Math::Mat3& matrix)
{
    return &matrix[0][0];
}

Orion::Math::Mat3 MakeTranslation2D(const Orion::Math::Vec2& translation)
{
    Orion::Math::Mat3 result{};
    result[0][2] = translation[0];
    result[1][2] = translation[1];
    return result;
}

Orion::Math::Mat3 MakeRotation2D(float angleRadians)
{
    const float cosine = std::cos(angleRadians);
    const float sine = std::sin(angleRadians);

    return Orion::Math::Mat3{
        Orion::Math::Vec3{ cosine, -sine, 0.0f },
        Orion::Math::Vec3{ sine, cosine, 0.0f },
        Orion::Math::Vec3{ 0.0f, 0.0f, 1.0f }
    };
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

    const std::string vertexShaderSource = R"(
        #version 460 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec4 aColor;

        uniform mat3 u_transform;

        out vec4 ourColor;

        void main()
        {
            vec3 transformed = u_transform * vec3(aPos, 1.0);
            gl_Position = vec4(transformed.xy, 0.0, 1.0);
            ourColor = aColor;
        }
    )";

    const std::string fragmentShaderSource = R"(
        #version 460 core
        in vec4 ourColor;
        out vec4 FragColor;

        void main()
        {
            FragColor = ourColor;
        }
    )";

    unsigned int shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (shaderProgram == 0)
    {
        glfwDestroyWindow(pWindow);
        glfwTerminate();
        return -1;
    }

    const float rotationAngle = Orion::Math::Trigonometry::Deg2Rad(90.0f);
    const Orion::Math::Vec2 nonUniformScale{ 0.5f, 1.5f };
    const Orion::Math::Vec2 topLeftPosition{ -0.5f, 0.5f };
    const Orion::Math::Vec2 bottomLeftPosition{ -0.5f, -0.5f };
    const Orion::Math::Vec2 topRightPosition{ 0.5f, 0.5f };
    const Orion::Math::Vec2 bottomRightPosition{ 0.5f, -0.5f };

    Orion::Math::Mat3 topLeft = MakeTranslation2D(topLeftPosition);

    Orion::Math::Mat3 bottomLeft = MakeTranslation2D(bottomLeftPosition);
    bottomLeft = bottomLeft * MakeRotation2D(rotationAngle);

    Orion::Math::Mat3 topRight = MakeTranslation2D(topRightPosition);
    topRight = Orion::Math::LinAlgOps::Scale(topRight, Orion::Math::Vec3{ nonUniformScale[0], nonUniformScale[1], 1.0f });

    Orion::Math::Mat3 bottomRight = MakeTranslation2D(bottomRightPosition);
    bottomRight = bottomRight * MakeRotation2D(rotationAngle);
    bottomRight = Orion::Math::LinAlgOps::Scale(bottomRight, Orion::Math::Vec3{ nonUniformScale[0], nonUniformScale[1], 1.0f });

    int transformLocation = glGetUniformLocation(shaderProgram, "u_transform");
    while (!glfwWindowShouldClose(pWindow))
    {
        glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], BACKGROUND_COLOR[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);

        glUniformMatrix3fv(transformLocation, 1, GL_TRUE, GetMatrixData(topLeft));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniformMatrix3fv(transformLocation, 1, GL_TRUE, GetMatrixData(bottomLeft));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniformMatrix3fv(transformLocation, 1, GL_TRUE, GetMatrixData(topRight));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniformMatrix3fv(transformLocation, 1, GL_TRUE, GetMatrixData(bottomRight));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    glfwDestroyWindow(pWindow);
    glfwTerminate();
}