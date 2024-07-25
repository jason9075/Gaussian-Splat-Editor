#pragma once
/*
/usr/local/include/glm/gtx/vector_angle.hpp:24:3: error: "GLM: GLM_GTX_vector_angle is an experimental
extension and may change in the future.
Use #define GLM_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
*/
#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <fstream>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
