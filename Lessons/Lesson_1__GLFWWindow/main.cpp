#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"


int main()
{
    if (!glfwInit())
    {
        spdlog::error("Failed to initialize GLFW");
        return -1;
    }

    GLFWwindow* pWindow = glfwCreateWindow(800, 600, "GLFW Window", nullptr, nullptr);
    if (!pWindow)
    {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(pWindow);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        spdlog::error("Failed to initialize GLAD");
        glfwDestroyWindow(pWindow);
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(pWindow))
    {
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glfwDestroyWindow(pWindow);
    glfwTerminate();
    return 0;
}