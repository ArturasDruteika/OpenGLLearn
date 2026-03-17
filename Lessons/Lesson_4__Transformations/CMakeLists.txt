cmake_minimum_required(VERSION 3.24)

project(Lesson_4__Transformations)

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
        Math
    Utils
)

# Ensure Example waits for Engine to build
add_dependencies(${PROJECT_NAME}
    Glad
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex.glsl
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment.glsl
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)
