#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"


constexpr int OPENGL_MAJOR_VERSION = 4;
constexpr int OPENGL_MINOR_VERSION = 6;
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float BACKGROUND_COLOR[4] = { 0.1f, 0.2f, 0.3f, 1.0f };


void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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
        // positions
         0.0f,  0.5f, 0.0f, // top vertex
        -0.5f, -0.5f, 0.0f, // bottom left
         0.5f, -0.5f, 0.0f  // bottom right
    };

    while (!glfwWindowShouldClose(pWindow))
    {
        glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], BACKGROUND_COLOR[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glfwDestroyWindow(pWindow);
    glfwTerminate();
}