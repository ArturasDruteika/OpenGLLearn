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

    GLFWwindow* p_window = glfwCreateWindow(800, 600, "GLFW Window", nullptr, nullptr);
    if (!p_window)
    {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(p_window);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        spdlog::error("Failed to initialize GLAD");
        glfwDestroyWindow(p_window);
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(p_window))
    {
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(p_window);
        glfwPollEvents();
    }

    glfwDestroyWindow(p_window);
    glfwTerminate();
    
    return 0;
}