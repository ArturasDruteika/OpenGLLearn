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
constexpr int OPENGL_MINOR_VERSION = 4;
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 800;
constexpr float BACKGROUND_COLOR[4] = { 0.1f, 0.2f, 0.3f, 1.0f };

constexpr float FOV_DEGREES_Y_AXIS = 45.0f;
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 100.0f;

// Global changable parameters
int g_framebufferWidth = WINDOW_WIDTH;
int g_framebufferHeight = WINDOW_HEIGHT;


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
    g_framebufferWidth = width;
    g_framebufferHeight = height;

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

const glm::mat4 CreateViewMatrix(
    const glm::vec3& cameraPosition,
    float cameraRotationAngleX,
    float cameraRotationAngleY,
    float cameraRotationAngleZ,
    float orbitRotationAngleX,
    float orbitRotationAngleY,
    float orbitRotationAngleZ
)
{
    // Camera local rotation matrices
    const glm::mat4 cameraRotateX3D = CreateRotationX3D(cameraRotationAngleX);
    const glm::mat4 cameraRotateY3D = CreateRotationY3D(cameraRotationAngleY);
    const glm::mat4 cameraRotateZ3D = CreateRotationZ3D(cameraRotationAngleZ);

    // Camera orbit rotation matrices
    const glm::mat4 orbitRotateX3D = CreateRotationX3D(orbitRotationAngleX);
    const glm::mat4 orbitRotateY3D = CreateRotationY3D(orbitRotationAngleY);
    const glm::mat4 orbitRotateZ3D = CreateRotationZ3D(orbitRotationAngleZ);

    // Local camera rotation: X first, then Y, then Z
    const glm::mat4 cameraRotation3D = cameraRotateZ3D * cameraRotateY3D * cameraRotateX3D;
    // Orbit rotation: X first, then Y, then Z
    const glm::mat4 orbitRotation3D = orbitRotateZ3D * orbitRotateY3D * orbitRotateX3D;
    // Create a single matrix comprised of camera and orbital rotations 
    const glm::mat4 finalCameraRotation3D = orbitRotation3D * cameraRotation3D;

    // Adjust position in real world
    const glm::vec3 newCameraPosition = glm::vec3(orbitRotation3D * glm::vec4(cameraPosition, 1.0f));

    // Calculate inverse camera rotation.
    // For an orthonormal rotation matrix, inverse(rotation) = transpose(rotation).
    const glm::mat3 viewRotation = glm::transpose(glm::mat3(finalCameraRotation3D));

    // Calculate inverse camera translation in the rotated coordinate system
    const glm::vec3 viewTranslation = viewRotation * (-newCameraPosition);

    // Calculate view matrix
    glm::mat4 view(1.0f);
    view[0] = glm::vec4(viewRotation[0], 0.0f);
    view[1] = glm::vec4(viewRotation[1], 0.0f);
    view[2] = glm::vec4(viewRotation[2], 0.0f);
    view[3] = glm::vec4(viewTranslation, 1.0f);

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

    std::string windowName = "3D Pyramid";

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

    glfwGetFramebufferSize(pWindow, &g_framebufferWidth, &g_framebufferHeight);
    glViewport(0, 0, g_framebufferWidth, g_framebufferHeight);

    // Pyramid is comprised of 5 vertices
    const glm::vec3 top = { 0.0f, 0.35f, 0.0f };
    const glm::vec3 frontLeft = { -0.25f, -0.25f, 0.25f };
    const glm::vec3 frontRight = { 0.25f, -0.25f, 0.25f };
    const glm::vec3 backLeft = { -0.25f, -0.25f, -0.25f };
    const glm::vec3 backRight = { 0.25f, -0.25f, -0.25f };

    // Each side of the pyramid will have it's own unique color
    const glm::vec4 neonBlue1 = { 0.0f, 0.3f, 0.8f, 1.0f };
    const glm::vec4 neonBlue2 = { 0.0f, 0.7f, 1.0f, 1.0f };
    const glm::vec4 neonGreen = { 0.2f, 1.0f, 0.2f, 1.0f };
    const glm::vec4 neonPurple = { 0.8f, 0.0f, 1.0f, 1.0f };
    const glm::vec4 baseBlue = { 0.0f, 0.4f, 0.8f, 1.0f };
    const glm::vec4 basePurple = { 0.4f, 0.0f, 0.8f, 1.0f };

    const std::vector<Vertex> vertices =
    {
        // Front face
        { top, neonBlue1 },        // 0: top
        { frontLeft, neonBlue2 },  // 1: front-left
        { frontRight, neonBlue2 }, // 2: front-right

        // Right face
        { top, neonBlue1 },        // 3: top
        { frontRight, neonGreen }, // 4: front-right
        { backRight, neonBlue2 },  // 5: back-right

        // Back face
        { top, neonPurple },       // 6: top
        { backRight, neonPurple }, // 7: back-right
        { backLeft, neonPurple },  // 8: back-left

        // Left face
        { top, neonBlue2 },        // 9: top
        { backLeft, neonBlue1 },   // 10: back-left
        { frontLeft, neonBlue1 },  // 11: front-left

        // Base face - square
        { frontLeft, baseBlue },    // 12: front-left
        { backLeft, baseBlue },     // 13: back-left
        { backRight, basePurple },  // 14: back-right
        { frontRight, basePurple }  // 15: front-right
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

    // Camera local angles
    const float cameraRotationAngleX = glm::radians(5.0f);   // pitch
    const float cameraRotationAngleY = glm::radians(-5.0f);   // yaw
    const float cameraRotationAngleZ = glm::radians(90.0f);  // roll

    // Camera orbit angles around origin
    const float orbitRotationAngleX = glm::radians(20.0f);
    const float orbitRotationAngleY = glm::radians(-35.0f);
    const float orbitRotationAngleZ = glm::radians(0.0f);

    // Camera base transform
    const glm::vec3 baseCameraPosition = { 0.0f, 0.0f, 2.5f };

    const glm::mat4 view = CreateViewMatrix(
        baseCameraPosition,
        cameraRotationAngleX,
        cameraRotationAngleY,
        cameraRotationAngleZ,
        orbitRotationAngleX,
        orbitRotationAngleY,
        orbitRotationAngleZ
    );

    const glm::vec3 scale = { 1.4f, 1.4f, 1.4f };
    const glm::mat4 scale3D = CreateScale3D(scale);
    const glm::mat4 model = scale3D;

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

        const float aspectRatio =
            static_cast<float>(g_framebufferWidth) /
            static_cast<float>(g_framebufferHeight > 0 ? g_framebufferHeight : 1);

        const glm::mat4 projection = glm::perspective(
            glm::radians(FOV_DEGREES_Y_AXIS),
            aspectRatio,
            NEAR_PLANE,
            FAR_PLANE
        );

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