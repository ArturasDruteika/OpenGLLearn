cmake_minimum_required(VERSION 3.24)

project(Utils)

set(CMAKE_BUILD_PARALLEL_LEVEL)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUTILS_EXPORT)

# ==== FileOperations ====
set(FILE_OPERATIONS_HEADERS
    Utils/FileOperations/include/FileOperations.hpp
)

set(FILE_OPERATIONS_SOURCES
    Utils/FileOperations/src/FileOperations.cpp
)

# Create a library target
add_library(${PROJECT_NAME} 
    SHARED
        ${FILE_OPERATIONS_HEADERS}
        ${FILE_OPERATIONS_SOURCES}
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Link spdlog
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        spdlog::spdlog
)


source_group(
    TREE 
        ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/FileOperations
    PREFIX
        "FileOperations"
    FILES
        ${FILE_OPERATIONS_HEADERS}
        ${FILE_OPERATIONS_SOURCES}
)
