#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"
#include <string>


constexpr int OPENGL_MAJOR_VERSION = 4;
constexpr int OPENGL_MINOR_VERSION = 6;
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float BACKGROUND_COLOR[4] = { 0.1f, 0.2f, 0.3f, 1.0f };


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int compile_shader(const std::string& source, unsigned int shader_id)
{
    const char* source_c_str = source.c_str();
    glShaderSource(shader_id, 1, &source_c_str, nullptr);
    glCompileShader(shader_id);

    int success;
    char info_log[512];
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader_id, 512, nullptr, info_log);
        spdlog::error("Shader compilation failed: {}", info_log);
    }
    return success;
}

unsigned int create_shader_program(const std::string& vertex_source, const std::string& fragment_source)
{
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    if (!compile_shader(vertex_source, vertex_shader))
    {
        glDeleteShader(vertex_shader);
        return 0;
    }

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compile_shader(fragment_source, fragment_shader))
    {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return 0;
    }

    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    int success;
    char info_log[512];
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
        spdlog::error("Shader program linking failed: {}", info_log);
        shader_program = 0;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
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

    GLFWwindow* p_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Triangle", nullptr, nullptr);
    if (!p_window)
    {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(p_window);
    glfwSetFramebufferSizeCallback(p_window, framebuffer_size_callback);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        spdlog::error("Failed to initialize GLAD");
        glfwDestroyWindow(p_window);
        glfwTerminate();
        return -1;
    }
    
    float triangle_vertices[] = {
        // positions    // colors
        -0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // top left
        -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom left
         0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom right
         0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // top right
    };

    unsigned int indices[] = { 
        0, 1, 2, // triangle 1
        2, 3, 0  // triangle 2
    };

    unsigned int vbo;
    unsigned int vao;
    unsigned int ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    const std::string vertex_shader_source = R"(
        #version 460 core
        layout (location = 0) in vec2 a_pos;
        layout (location = 1) in vec4 a_color;

        out vec4 our_color;

        void main()
        {
            gl_Position = vec4(a_pos, 0.0, 1.0);
            our_color = a_color;
        }
    )";

    const std::string fragment_shader_source = R"(
        #version 460 core
        in vec4 our_color;
        out vec4 frag_color;

        void main()
        {
            frag_color = our_color;
        }
    )";

    unsigned int shader_program = create_shader_program(vertex_shader_source, fragment_shader_source);
    if (shader_program == 0)
    {
        glfwDestroyWindow(p_window);
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(p_window))
    {
        glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], BACKGROUND_COLOR[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(p_window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    
    glfwDestroyWindow(p_window);
    glfwTerminate();
}