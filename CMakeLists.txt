cmake_minimum_required(VERSION 3.10)
project(MinecraftCpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

# Add executable
add_executable(minecraft
    src/main.cpp
    src/Chunk.cpp
    src/World.cpp
    src/Renderer.cpp
    src/ImageLoader.cpp
)

# Include directories
target_include_directories(minecraft PRIVATE
    include
    kernel
)

# Link libraries
if(APPLE)
    target_link_libraries(minecraft PRIVATE
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARY}
    )
else()
    target_link_libraries(minecraft PRIVATE
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARY}
        /usr/lib/x86_64-linux-gnu/libglut.so
        /usr/lib/x86_64-linux-gnu/libGL.so
        /usr/lib/x86_64-linux-gnu/libGLU.so
    )
endif()
