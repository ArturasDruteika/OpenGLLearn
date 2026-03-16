#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "Math/Constants/include/Constants.hpp"
#include "Math/LinearAlgebra/include/LinearAlgebraDataTypes.hpp"
#include "Math/LinearAlgebra/include/LinearAlgebraOperations.hpp"
#include "Math/Trigonometry/include/Trigonometry.hpp"
#include "spdlog/spdlog.h"
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

const float* GetMatrixData(const Orion::Math::Mat4& matrix)
{
    return &matrix[0][0];
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

        uniform mat4 uTransform;

        out vec4 ourColor;

        void main()
        {
            gl_Position = uTransform * vec4(aPos, 0.0, 1.0);
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
    const Orion::Math::Vec3 rotationAxis = Orion::Math::NEGATIVE_Z_AXIS;
    const Orion::Math::Vec3 nonUniformScale{ 0.5f, 1.5f, 1.0f };
    const Orion::Math::Vec3 topLeftPosition{ -0.5f, 0.5f, 0.0f };
    const Orion::Math::Vec3 bottomLeftPosition{ -0.5f, -0.5f, 0.0f };
    const Orion::Math::Vec3 topRightPosition{ 0.5f, 0.5f, 0.0f };
    const Orion::Math::Vec3 bottomRightPosition{ 0.5f, -0.5f, 0.0f };

    int transformLocation = glGetUniformLocation(shaderProgram, "uTransform");
    while (!glfwWindowShouldClose(pWindow))
    {
        glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], BACKGROUND_COLOR[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);

        Orion::Math::Mat4 topLeft = Orion::Math::LinAlgOps::Translate(topLeftPosition);
        glUniformMatrix4fv(transformLocation, 1, GL_TRUE, GetMatrixData(topLeft));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        Orion::Math::Mat4 bottomLeft = Orion::Math::LinAlgOps::Translate(Orion::Math::Mat4{}, bottomLeftPosition);
        bottomLeft = Orion::Math::LinAlgOps::Rotate(bottomLeft, rotationAngle, rotationAxis);
        glUniformMatrix4fv(transformLocation, 1, GL_TRUE, GetMatrixData(bottomLeft));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        Orion::Math::Mat4 topRight = Orion::Math::LinAlgOps::Translate(Orion::Math::Mat4{}, topRightPosition);
        topRight = Orion::Math::LinAlgOps::Scale(topRight, nonUniformScale);
        glUniformMatrix4fv(transformLocation, 1, GL_TRUE, GetMatrixData(topRight));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        Orion::Math::Mat4 bottomRight = Orion::Math::LinAlgOps::Translate(Orion::Math::Mat4{}, bottomRightPosition);
        bottomRight = Orion::Math::LinAlgOps::Rotate(bottomRight, rotationAngle, rotationAxis);
        bottomRight = Orion::Math::LinAlgOps::Scale(bottomRight, nonUniformScale);
        glUniformMatrix4fv(transformLocation, 1, GL_TRUE, GetMatrixData(bottomRight));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    glfwDestroyWindow(pWindow);
    glfwTerminate();
}