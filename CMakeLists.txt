cmake_minimum_required(VERSION 3.10)
project(OpenGLPlayground)

set(CMAKE_CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE "GLVND")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_package(GLEW 2.0 REQUIRED)
endif()
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)

# Include directories for external libraries
include_directories(include ${OPENGL_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} external/imgui)

# SDL2 and OpenGL libraries
set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/backends/imgui_impl_sdl2.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

file(GLOB SOURCES src/*.cpp)

# Disable tests and installation for nlohmann_json
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")

# Create imgui static library for target link
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PRIVATE external/imgui)

# Create playground executable
add_executable(playground.app Main.cpp ${SOURCES})

# Add precompiled header
target_precompile_headers(playground.app PRIVATE include/OPPCH.h)

# Link libraries
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(playground.app imgui ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} GLEW::GLEW nlohmann_json::nlohmann_json)
else()
    target_link_libraries(playground.app imgui ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} GLEW nlohmann_json::nlohmann_json)
endif()


