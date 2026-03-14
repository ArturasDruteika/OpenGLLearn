cmake_minimum_required(VERSION 3.24)

project(Lesson_1__GLFWWindow)

set(CMAKE_BUILD_PARALLEL_LEVEL)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp)

# OpenGL
find_package(OpenGL REQUIRED)
if (APPLE)
    # Ignore macOS OpenGL deprecation warnings
    target_compile_definitions(${PROJECT_NAME} PRIVATE GL_SILENCE_DEPRECATION)
endif ()

# Ensure Engine is linked
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Glad
        glfw
        OpenGL::GL
        spdlog::spdlog
)

# Ensure Example waits for Engine to build
add_dependencies(${PROJECT_NAME}
    Glad
)
