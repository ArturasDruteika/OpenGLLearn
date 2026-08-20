#include "utils/file_operations/include/file_operations.hpp"

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
constexpr int WINDOW_HEIGHT = 800;
constexpr float BACKGROUND_COLOR[4] = { 0.1f, 0.2f, 0.3f, 1.0f };


glm::mat3 create_translation_2d(const glm::vec2& translation)
{
    glm::mat3 result(1.0f);
    result[2] = glm::vec3(translation, 1.0f);
    return result;
}

glm::mat3 create_rotation_2d(float angle_in_radians)
{
    const float cosine = std::cos(angle_in_radians);
    const float sine = std::sin(angle_in_radians);

    glm::mat3 result(1.0f);
    result[0] = glm::vec3(cosine, sine, 0.0f);
    result[1] = glm::vec3(-sine, cosine, 0.0f);
    return result;
}

glm::mat3 create_scale_2d(const glm::vec2& scale)
{
    glm::mat3 result(1.0f);
    result[0] = glm::vec3(scale.x, 0.0f, 0.0f);
    result[1] = glm::vec3(0.0f, scale.y, 0.0f);
    return result;
}

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

    std::string window_name = "Triangle Transformations";

    GLFWwindow* p_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, window_name.c_str(), nullptr, nullptr);
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    std::string vertex_shader_source;
    std::string fragment_shader_source;

    std::filesystem::path vertex_shader_path = "shaders/vertex.glsl";
    std::filesystem::path fragment_shader_path = "shaders/fragment.glsl";

    try
    {
        vertex_shader_source = orion::utils::FileOperations::load_file_as_string(vertex_shader_path);
        fragment_shader_source = orion::utils::FileOperations::load_file_as_string(fragment_shader_path);
    }
    catch (const std::exception& exception)
    {
        spdlog::error("Failed to load shader files: {}", exception.what());
        glfwDestroyWindow(p_window);
        glfwTerminate();
        return -1;
    }

    unsigned int shader_program = create_shader_program(vertex_shader_source, fragment_shader_source);
    if (shader_program == 0)
    {
        glfwDestroyWindow(p_window);
        glfwTerminate();
        return -1;
    }

    const float rotation_angle = glm::radians(90.0f);
    const glm::vec2 non_uniform_scale{ 0.5f, 1.5f };
    const glm::vec2 top_left_position{ -0.5f, 0.5f };
    const glm::vec2 bottom_left_position{ -0.5f, -0.5f };
    const glm::vec2 top_right_position{ 0.5f, 0.5f };
    const glm::vec2 bottom_right_position{ 0.5f, -0.5f };

    const glm::mat3 translate_top_left = create_translation_2d(top_left_position);
    const glm::mat3 translate_bottom_left = create_translation_2d(bottom_left_position);
    const glm::mat3 translate_top_right = create_translation_2d(top_right_position);
    const glm::mat3 translate_bottom_right = create_translation_2d(bottom_right_position);

    const glm::mat3 rotate_2d = create_rotation_2d(rotation_angle);
    const glm::mat3 scale_2d = create_scale_2d(non_uniform_scale);

    const glm::mat3 top_left = translate_top_left;
    const glm::mat3 bottom_left = translate_bottom_left * rotate_2d;
    const glm::mat3 top_right = translate_top_right * scale_2d;
    const glm::mat3 bottom_right = translate_bottom_right * rotate_2d * scale_2d;

    int transform_location = glGetUniformLocation(shader_program, "u_transform");
    while (!glfwWindowShouldClose(p_window))
    {
        glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], BACKGROUND_COLOR[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);

        glUniformMatrix3fv(transform_location, 1, GL_FALSE, glm::value_ptr(top_left));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniformMatrix3fv(transform_location, 1, GL_FALSE, glm::value_ptr(bottom_left));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniformMatrix3fv(transform_location, 1, GL_FALSE, glm::value_ptr(top_right));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniformMatrix3fv(transform_location, 1, GL_FALSE, glm::value_ptr(bottom_right));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(p_window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    glfwDestroyWindow(p_window);
    glfwTerminate();

    return 0;
}