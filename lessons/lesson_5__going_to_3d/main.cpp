#include "utils/file_operations/include/file_operations.hpp"

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
constexpr int OPENGL_MINOR_VERSION = 4;
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 800;
constexpr float BACKGROUND_COLOR[4] = { 0.1f, 0.2f, 0.3f, 1.0f };

constexpr float FOV_DEGREES_Y_AXIS = 45.0f;
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 100.0f;

// Global changable parameters
int g_framebuffer_width = WINDOW_WIDTH;
int g_framebuffer_height = WINDOW_HEIGHT;


struct Vertex
{
    glm::vec3 position;
    glm::vec4 color;
};


glm::mat4 create_rotation_x3d(float angle_in_radians)
{
    const float cosine = std::cos(angle_in_radians);
    const float sine = std::sin(angle_in_radians);

    glm::mat4 result(1.0f);
    result[1][1] = cosine;
    result[1][2] = sine;
    result[2][1] = -sine;
    result[2][2] = cosine;
    return result;
}

glm::mat4 create_rotation_y3d(float angle_in_radians)
{
    const float cosine = std::cos(angle_in_radians);
    const float sine = std::sin(angle_in_radians);

    glm::mat4 result(1.0f);
    result[0][0] = cosine;
    result[0][2] = -sine;
    result[2][0] = sine;
    result[2][2] = cosine;
    return result;
}

glm::mat4 create_rotation_z3d(float angle_in_radians)
{
    const float cosine = std::cos(angle_in_radians);
    const float sine = std::sin(angle_in_radians);

    glm::mat4 result(1.0f);
    result[0][0] = cosine;
    result[0][1] = sine;
    result[1][0] = -sine;
    result[1][1] = cosine;
    return result;
}

glm::mat4 create_scale_3d(const glm::vec3& scale)
{
    glm::mat4 result(1.0f);
    result[0][0] = scale.x;
    result[1][1] = scale.y;
    result[2][2] = scale.z;
    return result;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    g_framebuffer_width = width;
    g_framebuffer_height = height;

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
        glDeleteProgram(shader_program);
        shader_program = 0;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

const glm::mat4 create_view_matrix(
    const glm::vec3& camera_position,
    float camera_rotation_angle_x,
    float camera_rotation_angle_y,
    float camera_rotation_angle_z,
    float orbit_rotation_angle_x,
    float orbit_rotation_angle_y,
    float orbit_rotation_angle_z
)
{
    // Camera local rotation matrices
    const glm::mat4 camera_rotate_x3d = create_rotation_x3d(camera_rotation_angle_x);
    const glm::mat4 camera_rotate_y3d = create_rotation_y3d(camera_rotation_angle_y);
    const glm::mat4 camera_rotate_z3d = create_rotation_z3d(camera_rotation_angle_z);

    // Camera orbit rotation matrices
    const glm::mat4 orbit_rotate_x3d = create_rotation_x3d(orbit_rotation_angle_x);
    const glm::mat4 orbit_rotate_y3d = create_rotation_y3d(orbit_rotation_angle_y);
    const glm::mat4 orbit_rotate_z3d = create_rotation_z3d(orbit_rotation_angle_z);

    // Local camera rotation: X first, then Y, then Z
    const glm::mat4 camera_rotation_3d = camera_rotate_z3d * camera_rotate_y3d * camera_rotate_x3d;
    // Orbit rotation: X first, then Y, then Z
    const glm::mat4 orbit_rotation_3d = orbit_rotate_z3d * orbit_rotate_y3d * orbit_rotate_x3d;
    // Create a single matrix comprised of camera and orbital rotations 
    const glm::mat4 final_camera_rotation_3d = orbit_rotation_3d * camera_rotation_3d;

    // Adjust position in real world
    const glm::vec3 new_camera_position = glm::vec3(orbit_rotation_3d * glm::vec4(camera_position, 1.0f));

    // Calculate inverse camera rotation.
    // For an orthonormal rotation matrix, inverse(rotation) = transpose(rotation).
    const glm::mat3 view_rotation = glm::transpose(glm::mat3(final_camera_rotation_3d));

    // Calculate inverse camera translation in the rotated coordinate system
    const glm::vec3 view_translation = view_rotation * (-new_camera_position);

    // Calculate view matrix
    glm::mat4 view(1.0f);
    view[0] = glm::vec4(view_rotation[0], 0.0f);
    view[1] = glm::vec4(view_rotation[1], 0.0f);
    view[2] = glm::vec4(view_rotation[2], 0.0f);
    view[3] = glm::vec4(view_translation, 1.0f);

    return view;
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

    std::string window_name = "3D Pyramid";

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

    glfwGetFramebufferSize(p_window, &g_framebuffer_width, &g_framebuffer_height);
    glViewport(0, 0, g_framebuffer_width, g_framebuffer_height);

    // Pyramid is comprised of 5 vertices
    const glm::vec3 top = { 0.0f, 0.35f, 0.0f };
    const glm::vec3 front_left = { -0.25f, -0.25f, 0.25f };
    const glm::vec3 front_right = { 0.25f, -0.25f, 0.25f };
    const glm::vec3 back_left = { -0.25f, -0.25f, -0.25f };
    const glm::vec3 back_right = { 0.25f, -0.25f, -0.25f };

    // Each side of the pyramid will have it's own unique color
    const glm::vec4 neon_blue_1 = { 0.0f, 0.3f, 0.8f, 1.0f };
    const glm::vec4 neon_blue_2 = { 0.0f, 0.7f, 1.0f, 1.0f };
    const glm::vec4 neon_green = { 0.2f, 1.0f, 0.2f, 1.0f };
    const glm::vec4 neon_purple = { 0.8f, 0.0f, 1.0f, 1.0f };
    const glm::vec4 base_blue = { 0.0f, 0.4f, 0.8f, 1.0f };
    const glm::vec4 base_purple = { 0.4f, 0.0f, 0.8f, 1.0f };

    const std::vector<Vertex> vertices =
    {
        // Front face
        { top, neon_blue_1 },        // 0: top
        { front_left, neon_blue_2 },  // 1: front-left
        { front_right, neon_blue_2 }, // 2: front-right

        // Right face
        { top, neon_blue_1 },        // 3: top
        { front_right, neon_green }, // 4: front-right
        { back_right, neon_blue_2 },  // 5: back-right

        // Back face
        { top, neon_purple },       // 6: top
        { back_right, neon_purple }, // 7: back-right
        { back_left, neon_purple },  // 8: back-left

        // Left face
        { top, neon_blue_2 },        // 9: top
        { back_left, neon_blue_1 },   // 10: back-left
        { front_left, neon_blue_1 },  // 11: front-left

        // Base face - square
        { front_left, base_blue },    // 12: front-left
        { back_left, base_blue },     // 13: back-left
        { back_right, base_purple },  // 14: back-right
        { front_right, base_purple }  // 15: front-right
    };

    const std::vector<unsigned int> indices =
    {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,

        12, 13, 14,
        12, 14, 15
    };

    glEnable(GL_DEPTH_TEST);

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

    std::string vertex_shader_source;
    std::string fragment_shader_source;

    std::filesystem::path executable_directory = std::filesystem::canonical("/proc/self/exe").parent_path();
    std::filesystem::path vertex_shader_path = executable_directory / "shaders/vertex.glsl";
    std::filesystem::path fragment_shader_path = executable_directory / "shaders/fragment.glsl";

    try
    {
        vertex_shader_source = orion::utils::FileOperations::load_file_as_string(vertex_shader_path);
        fragment_shader_source = orion::utils::FileOperations::load_file_as_string(fragment_shader_path);
    }
    catch (const std::exception& exception)
    {
        spdlog::error("Failed to load shader files: {}", exception.what());
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glfwDestroyWindow(p_window);
        glfwTerminate();
        return -1;
    }

    unsigned int shader_program = create_shader_program(vertex_shader_source, fragment_shader_source);
    if (shader_program == 0)
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glfwDestroyWindow(p_window);
        glfwTerminate();
        return -1;
    }

    // Camera local angles
    const float camera_rotation_angle_x = glm::radians(5.0f);   // pitch
    const float camera_rotation_angle_y = glm::radians(-5.0f);   // yaw
    const float camera_rotation_angle_z = glm::radians(90.0f);  // roll

    // Camera orbit angles around origin
    const float orbit_rotation_angle_x = glm::radians(20.0f);
    const float orbit_rotation_angle_y = glm::radians(-35.0f);
    const float orbit_rotation_angle_z = glm::radians(0.0f);

    // Camera base transform
    const glm::vec3 base_camera_position = { 0.0f, 0.0f, 2.5f };

    const glm::mat4 view = create_view_matrix(
        base_camera_position,
        camera_rotation_angle_x,
        camera_rotation_angle_y,
        camera_rotation_angle_z,
        orbit_rotation_angle_x,
        orbit_rotation_angle_y,
        orbit_rotation_angle_z
    );

    const glm::vec3 scale = { 1.4f, 1.4f, 1.4f };
    const glm::mat4 scale_3d = create_scale_3d(scale);
    const glm::mat4 model = scale_3d;

    int mvp_location = glGetUniformLocation(shader_program, "u_mvp");
    if (mvp_location == -1)
    {
        spdlog::error("Failed to find uniform location for u_mvp");
        glDeleteProgram(shader_program);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glfwDestroyWindow(p_window);
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(p_window))
    {
        glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], BACKGROUND_COLOR[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);

        const float aspect_ratio =
            static_cast<float>(g_framebuffer_width) /
            static_cast<float>(g_framebuffer_height > 0 ? g_framebuffer_height : 1);

        const glm::mat4 projection = glm::perspective(
            glm::radians(FOV_DEGREES_Y_AXIS),
            aspect_ratio,
            NEAR_PLANE,
            FAR_PLANE
        );

        const glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(indices.size()),
            GL_UNSIGNED_INT,
            nullptr
        );

        glfwSwapBuffers(p_window);
        glfwPollEvents();
    }

    glDeleteProgram(shader_program);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glfwDestroyWindow(p_window);
    glfwTerminate();

    return 0;
}