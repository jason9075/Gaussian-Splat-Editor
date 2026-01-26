#include "App.hpp"

#include <OPPCH.h>

#include "ImGuiFileDialog.h"

App::App(const float screenWidth, const float screenHeight) {
  width = screenWidth;
  height = screenHeight;
  tan_fovx = tan(glm::radians(45.0f) / 2.0f);
  tan_fovy = tan(glm::radians(45.0f) / 2.0f);
  focal_y = width / (2.0f * tan_fovy);
  focal_x = height / (2.0f * tan_fovx);
  glViewport(0, 0, screenWidth, screenHeight);

  splatShader =
      std::make_unique<Shader>("./shaders/geo_vert.glsl", "./shaders/geo_gert.glsl", "./shaders/geo_frag.glsl");

  splat = loadPlyFile(modelPath);

  glm::vec3 position = glm::vec3(5.0f, 3.0f, 0.0f);
  glm::vec3 orientation = glm::vec3(-0.7f, -0.6f, 0.0f);
  camera = std::make_unique<Camera>(screenWidth, screenHeight, position, orientation);
  listener = std::make_unique<GhostCameraListener>(camera.get());
  listener->onKeyUpCallback = [&](SDL_Keycode key) { splat->sort(getViewModelMatrix()); };
  listener->onMouseLeftReleaseCallback = [&]() { splat->sort(getViewModelMatrix()); };
  camera->setEventListener(listener.get());
  splat->sort(getViewModelMatrix());

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  // Box Mesh (24 vertices for flat shading)
  boxShader = std::make_unique<Shader>("./shaders/box_vert.glsl", "./shaders/box_frag.glsl");
  std::vector<Vertex> boxVertices = {// Front face
                                     {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                                     {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                                     {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                                     {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                                     // Back face
                                     {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},
                                     {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},
                                     {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},
                                     {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},
                                     // Top face
                                     {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
                                     {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
                                     {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
                                     {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
                                     // Bottom face
                                     {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}},
                                     {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}},
                                     {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
                                     {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
                                     // Right face
                                     {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
                                     {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
                                     {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
                                     {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
                                     // Left face
                                     {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}},
                                     {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
                                     {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
                                     {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}}};
  std::vector<GLuint> boxIndices;
  for (int i = 0; i < 6; ++i) {
    boxIndices.push_back(i * 4 + 0);
    boxIndices.push_back(i * 4 + 1);
    boxIndices.push_back(i * 4 + 2);
    boxIndices.push_back(i * 4 + 2);
    boxIndices.push_back(i * 4 + 3);
    boxIndices.push_back(i * 4 + 0);
  }
  boxMesh = std::make_unique<Mesh>(boxVertices, boxIndices);

  // Sphere Mesh for Gizmo
  std::vector<Vertex> sphereVertices;
  std::vector<GLuint> sphereIndices;
  const int sectors = 20;
  const int stacks = 20;
  const float PI = 3.14159265f;
  for (int i = 0; i <= stacks; ++i) {
    float stackAngle = PI / 2 - i * PI / stacks;
    float xy = cosf(stackAngle);
    float z = sinf(stackAngle);
    for (int j = 0; j <= sectors; ++j) {
      float sectorAngle = j * 2 * PI / sectors;
      float x = xy * cosf(sectorAngle);
      float y = xy * sinf(sectorAngle);
      sphereVertices.push_back({{x, y, z}, {x, y, z}, {1.0f, 1.0f, 1.0f}});
    }
  }
  for (int i = 0; i < stacks; ++i) {
    int k1 = i * (sectors + 1);
    int k2 = k1 + sectors + 1;
    for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
      if (i != 0) {
        sphereIndices.push_back(k1);
        sphereIndices.push_back(k2);
        sphereIndices.push_back(k1 + 1);
      }
      if (i != (stacks - 1)) {
        sphereIndices.push_back(k1 + 1);
        sphereIndices.push_back(k2);
        sphereIndices.push_back(k2 + 1);
      }
    }
  }
  sphereMesh = std::make_unique<Mesh>(sphereVertices, sphereIndices);

  // Cone Mesh for Arrows
  std::vector<Vertex> coneVertices;
  std::vector<GLuint> coneIndices;
  const int coneSectors = 20;
  coneVertices.push_back({{0, 0, 1.0f}, {0, 0, 1.0f}, {1.0f, 1.0f, 1.0f}});  // Tip
  for (int i = 0; i <= coneSectors; ++i) {
    float angle = i * 2 * PI / coneSectors;
    float x = cosf(angle);
    float y = sinf(angle);
    coneVertices.push_back({{x, y, 0}, {x, y, 0}, {1.0f, 1.0f, 1.0f}});
  }
  for (int i = 1; i <= coneSectors; ++i) {
    coneIndices.push_back(0);
    coneIndices.push_back(i);
    coneIndices.push_back(i + 1);
  }
  // Bottom cap
  int centerIdx = coneVertices.size();
  coneVertices.push_back({{0, 0, 0}, {0, 0, -1.0f}, {1.0f, 1.0f, 1.0f}});
  for (int i = 1; i <= coneSectors; ++i) {
    coneIndices.push_back(centerIdx);
    coneIndices.push_back(i + 1);
    coneIndices.push_back(i);
  }
  coneMesh = std::make_unique<Mesh>(coneVertices, coneIndices);
}
App::~App() {}

glm::vec3 App::getArcballVector(float x, float y) {
  // Map to [-1, 1]
  float mx = (x - (width - gizmoSize)) / gizmoSize * 2.0f - 1.0f;
  float my = (y - (height - gizmoSize)) / gizmoSize * 2.0f - 1.0f;
  my = -my;  // Invert Y for screen space

  glm::vec3 p = glm::vec3(mx, my, 0.0f);
  float mag2 = mx * mx + my * my;
  if (mag2 <= 1.0f) {
    p.z = sqrt(1.0f - mag2);
  } else {
    p = glm::normalize(p);
  }
  return p;
}

glm::vec3 App::getMouseRay(float x, float y) {
  float nx = (2.0f * x) / width - 1.0f;
  float ny = 1.0f - (2.0f * y) / height;
  glm::vec4 ray_clip = glm::vec4(nx, ny, -1.0f, 1.0f);
  glm::vec4 ray_eye = glm::inverse(camera->projMatrix) * ray_clip;
  ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
  glm::vec3 ray_wor = glm::vec3(glm::inverse(camera->viewMatrix) * ray_eye);
  return glm::normalize(ray_wor);
}

bool App::intersectRaySphere(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& sphereCenter,
                             float radius, float& t) {
  glm::vec3 oc = rayOrigin - sphereCenter;
  float b = glm::dot(oc, rayDir);
  float c = glm::dot(oc, oc) - radius * radius;
  float h = b * b - c;
  if (h < 0.0f) return false;
  h = sqrt(h);
  t = -b - h;
  if (t < 0) t = -b + h;
  return t >= 0;
}

void App::OnEvent(SDL_Event& event) {
  if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
    float mx = event.button.x;
    float my = event.button.y;

    // 1. Check Rotation Gizmo (Bottom Right)
    if (mx > (width - gizmoSize) && my > (height - gizmoSize)) {
      isRotating = true;
      lastMousePos = glm::vec2(mx, my);
      return;
    }

    // 2. Check Selection Box Gizmos (Translation Arrows & Scaling Handles)
    glm::vec3 rayOrigin = camera->position;
    glm::vec3 rayDir = getMouseRay(mx, my);
    glm::vec3 boxPos = glm::vec3(boxPosition[0], boxPosition[1], boxPosition[2]);
    float handleRadius = 0.3f;
    float minT = 1e10f;
    currentGizmoMode = GizmoMode::NONE;

    // Translation Handles (Arrows)
    float t;
    if (intersectRaySphere(rayOrigin, rayDir, boxPos + glm::vec3(2.0f, 0, 0), 0.5f, t) && t < minT) {
      minT = t;
      currentGizmoMode = GizmoMode::TRANSLATE_X;
    }
    if (intersectRaySphere(rayOrigin, rayDir, boxPos + glm::vec3(0, 2.0f, 0), 0.5f, t) && t < minT) {
      minT = t;
      currentGizmoMode = GizmoMode::TRANSLATE_Y;
    }
    if (intersectRaySphere(rayOrigin, rayDir, boxPos + glm::vec3(0, 0, 2.0f), 0.5f, t) && t < minT) {
      minT = t;
      currentGizmoMode = GizmoMode::TRANSLATE_Z;
    }

    // Scaling Handles (Faces)
    if (intersectRaySphere(rayOrigin, rayDir, boxPos + glm::vec3(boxSize[0], 0, 0), handleRadius, t) && t < minT) {
      minT = t;
      currentGizmoMode = GizmoMode::SCALE_PX;
    }
    if (intersectRaySphere(rayOrigin, rayDir, boxPos + glm::vec3(-boxSize[0], 0, 0), handleRadius, t) && t < minT) {
      minT = t;
      currentGizmoMode = GizmoMode::SCALE_NX;
    }
    if (intersectRaySphere(rayOrigin, rayDir, boxPos + glm::vec3(0, boxSize[1], 0), handleRadius, t) && t < minT) {
      minT = t;
      currentGizmoMode = GizmoMode::SCALE_PY;
    }
    if (intersectRaySphere(rayOrigin, rayDir, boxPos + glm::vec3(0, -boxSize[1], 0), handleRadius, t) && t < minT) {
      minT = t;
      currentGizmoMode = GizmoMode::SCALE_NY;
    }
    if (intersectRaySphere(rayOrigin, rayDir, boxPos + glm::vec3(0, 0, boxSize[2]), handleRadius, t) && t < minT) {
      minT = t;
      currentGizmoMode = GizmoMode::SCALE_PZ;
    }
    if (intersectRaySphere(rayOrigin, rayDir, boxPos + glm::vec3(0, 0, -boxSize[2]), handleRadius, t) && t < minT) {
      minT = t;
      currentGizmoMode = GizmoMode::SCALE_NZ;
    }

    if (currentGizmoMode != GizmoMode::NONE) {
      dragStartWorldPos = rayOrigin + rayDir * minT;
      dragStartBoxPos = boxPos;
      dragStartBoxSize = glm::vec3(boxSize[0], boxSize[1], boxSize[2]);
      return;
    }

  } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
    if (isRotating) splat->sort(getViewModelMatrix());
    isRotating = false;
    currentGizmoMode = GizmoMode::NONE;
  } else if (event.type == SDL_MOUSEMOTION) {
    float mx = event.motion.x;
    float my = event.motion.y;

    if (isRotating) {
      glm::vec3 va = getArcballVector(lastMousePos.x, lastMousePos.y);
      glm::vec3 vb = getArcballVector(mx, my);
      if (va != vb) {
        float angle = acos(glm::clamp(glm::dot(va, vb), -1.0f, 1.0f));
        glm::vec3 axis = glm::cross(va, vb);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle * 2.0f, axis);
        splatModelMatrix = rotation * splatModelMatrix;
      }
      lastMousePos = glm::vec2(mx, my);
    } else if (currentGizmoMode != GizmoMode::NONE) {
      glm::vec3 rayOrigin = camera->position;
      glm::vec3 rayDir = getMouseRay(mx, my);

      // Axis Projection for Translation
      if (currentGizmoMode == GizmoMode::TRANSLATE_X || currentGizmoMode == GizmoMode::TRANSLATE_Y ||
          currentGizmoMode == GizmoMode::TRANSLATE_Z) {
        glm::vec3 axis(0);
        if (currentGizmoMode == GizmoMode::TRANSLATE_X) axis = glm::vec3(1, 0, 0);
        if (currentGizmoMode == GizmoMode::TRANSLATE_Y) axis = glm::vec3(0, 1, 0);
        if (currentGizmoMode == GizmoMode::TRANSLATE_Z) axis = glm::vec3(0, 0, 1);

        // Plane for projection: cross of axis and (axis cross cameraDir)
        glm::vec3 planeNormal = glm::normalize(glm::cross(axis, glm::cross(axis, rayOrigin - dragStartBoxPos)));
        float t = glm::dot(dragStartWorldPos - rayOrigin, planeNormal) / glm::dot(rayDir, planeNormal);
        glm::vec3 currentWorldPos = rayOrigin + rayDir * t;
        float offset = glm::dot(currentWorldPos - dragStartWorldPos, axis);

        boxPosition[0] = dragStartBoxPos.x + (currentGizmoMode == GizmoMode::TRANSLATE_X ? offset : 0);
        boxPosition[1] = dragStartBoxPos.y + (currentGizmoMode == GizmoMode::TRANSLATE_Y ? offset : 0);
        boxPosition[2] = dragStartBoxPos.z + (currentGizmoMode == GizmoMode::TRANSLATE_Z ? offset : 0);
      }
      // Axis Projection for Scaling
      else {
        glm::vec3 axis(0);
        int idx = 0;
        float sign = 1.0f;
        if (currentGizmoMode == GizmoMode::SCALE_PX) {
          axis = glm::vec3(1, 0, 0);
          idx = 0;
          sign = 1;
        }
        if (currentGizmoMode == GizmoMode::SCALE_NX) {
          axis = glm::vec3(1, 0, 0);
          idx = 0;
          sign = -1;
        }
        if (currentGizmoMode == GizmoMode::SCALE_PY) {
          axis = glm::vec3(0, 1, 0);
          idx = 1;
          sign = 1;
        }
        if (currentGizmoMode == GizmoMode::SCALE_NY) {
          axis = glm::vec3(0, 1, 0);
          idx = 1;
          sign = -1;
        }
        if (currentGizmoMode == GizmoMode::SCALE_PZ) {
          axis = glm::vec3(0, 0, 1);
          idx = 2;
          sign = 1;
        }
        if (currentGizmoMode == GizmoMode::SCALE_NZ) {
          axis = glm::vec3(0, 0, 1);
          idx = 2;
          sign = -1;
        }

        glm::vec3 planeNormal = glm::normalize(glm::cross(axis, glm::cross(axis, rayOrigin - dragStartWorldPos)));
        float t = glm::dot(dragStartWorldPos - rayOrigin, planeNormal) / glm::dot(rayDir, planeNormal);
        glm::vec3 currentWorldPos = rayOrigin + rayDir * t;
        float offset = glm::dot(currentWorldPos - dragStartWorldPos, axis);

        boxSize[idx] = std::max(0.1f, dragStartBoxSize[idx] + offset * sign * 0.5f);
        boxPosition[idx] = dragStartBoxPos[idx] + offset * 0.5f;
      }
    }
  }

  if (!isRotating && currentGizmoMode == GizmoMode::NONE) {
    camera->handle(event);
  }
}

void App::OnRender() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  camera->moveCamera();

  // Use premultiplied alpha for Gaussian Splats
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  splatShader->use();
  glUniformMatrix4fv(glGetUniformLocation(splatShader->ID, "modelMatrix"), 1, GL_FALSE,
                     glm::value_ptr(splatModelMatrix));
  glUniform1f(glGetUniformLocation(splatShader->ID, "scaleFactor"), scaleFactor);
  glUniform2f(glGetUniformLocation(splatShader->ID, "Resolution"), (float)width, (float)height);
  glUniform2f(glGetUniformLocation(splatShader->ID, "Focal"), focal_x, focal_y);
  glUniform2f(glGetUniformLocation(splatShader->ID, "TanFov"), tan_fovx, tan_fovy);
  // calc box bounds location
  glm::vec3 boxMin = glm::vec3(boxPosition[0] - boxSize[0], boxPosition[1] - boxSize[1], boxPosition[2] - boxSize[2]);
  glm::vec3 boxMax = glm::vec3(boxPosition[0] + boxSize[0], boxPosition[1] + boxSize[1], boxPosition[2] + boxSize[2]);
  glUniform3fv(glGetUniformLocation(splatShader->ID, "BoxMin"), 1, glm::value_ptr(boxMin));
  glUniform3fv(glGetUniformLocation(splatShader->ID, "BoxMax"), 1, glm::value_ptr(boxMax));
  glUniform1i(glGetUniformLocation(splatShader->ID, "showSelectionBox"), showSelectionBox ? 1 : 0);
  camera->update(splatShader.get());
  splat->draw(splatShader.get());

  // Use standard alpha blending for GUI/Gizmos
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (showSelectionBox) {
    boxShader->use();
    // Selection Box should be axis-aligned in world space, independent of model rotation
    glUniformMatrix4fv(glGetUniformLocation(boxShader->ID, "modelMatrix"), 1, GL_FALSE,
                       glm::value_ptr(glm::mat4(1.0f)));
    camera->update(boxShader.get());
    glUniform3f(glGetUniformLocation(boxShader->ID, "Color"), 0.8f, 0.8f, 0.8f);
    glUniform3fv(glGetUniformLocation(boxShader->ID, "Size"), 1, boxSize);
    glUniform3fv(glGetUniformLocation(boxShader->ID, "Position"), 1, boxPosition);
    boxMesh->draw(boxShader.get());

    // Wireframe overlay for sharp edges
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUniform3f(glGetUniformLocation(boxShader->ID, "Color"), 1.0f, 1.0f, 1.0f);
    boxMesh->draw(boxShader.get());
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // Render Rotation Gizmo in corner
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(width - (int)gizmoSize, 0, (int)gizmoSize, (int)gizmoSize);
  boxShader->use();
  // Simple identity projection/view for gizmo
  glm::mat4 gizmoProj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);
  glm::mat4 gizmoView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
  glUniformMatrix4fv(glGetUniformLocation(boxShader->ID, "camMatrix"), 1, GL_FALSE,
                     glm::value_ptr(gizmoProj * gizmoView));
  glUniformMatrix4fv(glGetUniformLocation(boxShader->ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(splatModelMatrix));

  // Draw X axis (Red)
  glUniform3f(glGetUniformLocation(boxShader->ID, "Color"), 1.0f, 0.0f, 0.0f);
  glUniform3f(glGetUniformLocation(boxShader->ID, "Size"), 1.0f, 0.05f, 0.05f);
  glUniform3f(glGetUniformLocation(boxShader->ID, "Position"), 0.5f, 0.0f, 0.0f);
  boxMesh->draw(boxShader.get());
  // Draw Y axis (Green)
  glUniform3f(glGetUniformLocation(boxShader->ID, "Color"), 0.0f, 1.0f, 0.0f);
  glUniform3f(glGetUniformLocation(boxShader->ID, "Size"), 0.05f, 1.0f, 0.05f);
  glUniform3f(glGetUniformLocation(boxShader->ID, "Position"), 0.0f, 0.5f, 0.0f);
  boxMesh->draw(boxShader.get());
  // Draw Z axis (Blue)
  glUniform3f(glGetUniformLocation(boxShader->ID, "Color"), 0.0f, 0.0f, 1.0f);
  glUniform3f(glGetUniformLocation(boxShader->ID, "Size"), 0.05f, 0.05f, 1.0f);
  glUniform3f(glGetUniformLocation(boxShader->ID, "Position"), 0.0f, 0.0f, 0.5f);
  boxMesh->draw(boxShader.get());

  // Draw Trackball Sphere
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glUniform3f(glGetUniformLocation(boxShader->ID, "Color"), 0.6f, 0.6f, 0.6f);
  glUniform3f(glGetUniformLocation(boxShader->ID, "Size"), 1.0f, 1.0f, 1.0f);
  glUniform3f(glGetUniformLocation(boxShader->ID, "Position"), 0.0f, 0.0f, 0.0f);
  glUniformMatrix4fv(glGetUniformLocation(boxShader->ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(splatModelMatrix));
  sphereMesh->draw(boxShader.get());
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glViewport(0, 0, (int)width, (int)height);

  // --- Render 3D Gizmo Handles ---
  if (showSelectionBox) {
    boxShader->use();
    camera->update(boxShader.get());
    glUniformMatrix4fv(glGetUniformLocation(boxShader->ID, "modelMatrix"), 1, GL_FALSE,
                       glm::value_ptr(glm::mat4(1.0f)));
    glm::vec3 boxPos = glm::vec3(boxPosition[0], boxPosition[1], boxPosition[2]);

    // Translation Arrows (Using Cones)
    // X Arrow (Red)
    glUniform3f(glGetUniformLocation(boxShader->ID, "Color"), 1.0f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(boxShader->ID, "Size"), 0.2f, 0.2f, 0.5f);
    glm::mat4 modelX = glm::translate(glm::mat4(1.0f), boxPos + glm::vec3(2.0f, 0, 0));
    modelX = glm::rotate(modelX, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(glGetUniformLocation(boxShader->ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelX));
    coneMesh->draw(boxShader.get());

    // Y Arrow (Green)
    glUniform3f(glGetUniformLocation(boxShader->ID, "Color"), 0.2f, 1.0f, 0.2f);
    glm::mat4 modelY = glm::translate(glm::mat4(1.0f), boxPos + glm::vec3(0, 2.0f, 0));
    modelY = glm::rotate(modelY, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    glUniformMatrix4fv(glGetUniformLocation(boxShader->ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelY));
    coneMesh->draw(boxShader.get());

    // Z Arrow (Blue)
    glUniform3f(glGetUniformLocation(boxShader->ID, "Color"), 0.2f, 0.2f, 1.0f);
    glm::mat4 modelZ = glm::translate(glm::mat4(1.0f), boxPos + glm::vec3(0, 0, 2.0f));
    glUniformMatrix4fv(glGetUniformLocation(boxShader->ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelZ));
    coneMesh->draw(boxShader.get());

    // Scaling Handles (Small cyan spheres on faces) - Size decreased from 0.2 to 0.1
    glUniform3f(glGetUniformLocation(boxShader->ID, "Color"), 0.0f, 1.0f, 1.0f);  // Cyan
    glUniform3f(glGetUniformLocation(boxShader->ID, "Size"), 0.1f, 0.1f, 0.1f);
    glUniformMatrix4fv(glGetUniformLocation(boxShader->ID, "modelMatrix"), 1, GL_FALSE,
                       glm::value_ptr(glm::mat4(1.0f)));
    glUniform3fv(glGetUniformLocation(boxShader->ID, "Position"), 1,
                 glm::value_ptr(boxPos + glm::vec3(boxSize[0], 0, 0)));
    sphereMesh->draw(boxShader.get());
    glUniform3fv(glGetUniformLocation(boxShader->ID, "Position"), 1,
                 glm::value_ptr(boxPos + glm::vec3(-boxSize[0], 0, 0)));
    sphereMesh->draw(boxShader.get());
    glUniform3fv(glGetUniformLocation(boxShader->ID, "Position"), 1,
                 glm::value_ptr(boxPos + glm::vec3(0, boxSize[1], 0)));
    sphereMesh->draw(boxShader.get());
    glUniform3fv(glGetUniformLocation(boxShader->ID, "Position"), 1,
                 glm::value_ptr(boxPos + glm::vec3(0, -boxSize[1], 0)));
    sphereMesh->draw(boxShader.get());
    glUniform3fv(glGetUniformLocation(boxShader->ID, "Position"), 1,
                 glm::value_ptr(boxPos + glm::vec3(0, 0, boxSize[2])));
    sphereMesh->draw(boxShader.get());
    glUniform3fv(glGetUniformLocation(boxShader->ID, "Position"), 1,
                 glm::value_ptr(boxPos + glm::vec3(0, 0, -boxSize[2])));
    sphereMesh->draw(boxShader.get());
  }

  glEnable(GL_DEPTH_TEST);
}

void App::OnImGuiRender() {
  ImGui::Text("Camera Position:");
  ImGui::Text("X:%.2f Y:%.2f Z:%.2f", camera->position.x, camera->position.y, camera->position.z);
  ImGui::Text("Camera Orientation:");
  ImGui::Text("X:%.2f Y:%.2f Z:%.2f", camera->orientation.x, camera->orientation.y, camera->orientation.z);
  showPathSelector();

  ImGui::SliderFloat("ScaleF", &scaleFactor, 0.1f, 3.0f);

  ImGui::Checkbox("Show Selection Box", &showSelectionBox);

  if (showSelectionBox) {
    if (ImGui::Button("Remove Splat")) {
      splat->removeSplats(boxPosition, boxSize, splatModelMatrix);
      splat->sort(getViewModelMatrix());
    }
  }

  if (ImGui::Button("Save Output.ply")) {
    savePlyFile("./assets/output.ply");
  }
}

glm::mat4 App::getViewModelMatrix() { return camera->viewMatrix * splatModelMatrix; }

std::unique_ptr<GaussianSplat> App::loadPlyFile(std::string path) {
  std::vector<GaussianSphere> spheres;
  if (path.empty()) {
    return std::make_unique<GaussianSplat>(spheres);
  }
  happly::PLYData plyIn(path);
  printInfo(plyIn);

  auto x = plyIn.getElement("vertex").getProperty<float>("x");
  auto y = plyIn.getElement("vertex").getProperty<float>("y");
  auto z = plyIn.getElement("vertex").getProperty<float>("z");
  // https://github.com/graphdeco-inria/gaussian-splatting/issues/485
  auto red = plyIn.getElement("vertex").getProperty<float>("f_dc_0");
  auto grn = plyIn.getElement("vertex").getProperty<float>("f_dc_1");
  auto blu = plyIn.getElement("vertex").getProperty<float>("f_dc_2");

  auto scaleX = plyIn.getElement("vertex").getProperty<float>("scale_0");
  auto scaleY = plyIn.getElement("vertex").getProperty<float>("scale_1");
  auto scaleZ = plyIn.getElement("vertex").getProperty<float>("scale_2");

  auto rotate0 = plyIn.getElement("vertex").getProperty<float>("rot_0");
  auto rotate1 = plyIn.getElement("vertex").getProperty<float>("rot_1");
  auto rotate2 = plyIn.getElement("vertex").getProperty<float>("rot_2");
  auto rotate3 = plyIn.getElement("vertex").getProperty<float>("rot_3");

  auto opacity = plyIn.getElement("vertex").getProperty<float>("opacity");

  for (int i = 0; i < x.size(); i++) {
    GaussianSphere sphere;
    sphere.position = glm::vec3(x[i], y[i], z[i]);
    sphere.color = glm::vec3(0.5f + C0 * red[i], 0.5f + C0 * grn[i], 0.5f + C0 * blu[i]);  // normalize color
    glm::mat3 R(glm::normalize(glm::quat(rotate0[i], rotate1[i], rotate2[i], rotate3[i])));
    glm::mat3 S =
        glm::mat3(std::exp(scaleX[i]), 0.0f, 0.0f, 0.0f, std::exp(scaleY[i]), 0.0f, 0.0f, 0.0f, std::exp(scaleZ[i]));
    glm::mat3 M = R * S * glm::transpose(S) * glm::transpose(R);
    sphere.covA = glm::vec3(M[0][0], M[0][1], M[0][2]);
    sphere.covB = glm::vec3(M[1][1], M[1][2], M[2][2]);
    sphere.opacity = 1. / (1. + std::exp(-opacity[i]));
    sphere.index = i;
    spheres.push_back(sphere);
  }
  return std::make_unique<GaussianSplat>(spheres);
}

void App::savePlyFile(const std::string outputPath) {
  if (outputPath.empty() || modelPath.empty()) {
    return;
  }
  // load the input ply file
  happly::PLYData plyIn(modelPath);
  std::vector<int> existingIndices = splat->getExistingIndices();

  std::cout << "length of existing indices: " << existingIndices.size() << std::endl;

  happly::PLYData plyOut;
  plyOut.comments = plyIn.comments;
  plyOut.objInfoComments = plyIn.objInfoComments;

  auto elementNames = plyIn.getElementNames();

  // Prepare rotation baking
  glm::quat bakeRot = glm::quat_cast(splatModelMatrix);
  glm::mat3 bakeRotMat = glm::mat3_cast(bakeRot);

  for (const auto& elementName : elementNames) {
    auto properties = plyIn.getElement(elementName).getPropertyNames();
    plyOut.addElement(elementName, existingIndices.size());

    if (elementName == "vertex") {
      // Handle vertex transformation separately
      std::vector<float> px = plyIn.getElement("vertex").getProperty<float>("x");
      std::vector<float> py = plyIn.getElement("vertex").getProperty<float>("y");
      std::vector<float> pz = plyIn.getElement("vertex").getProperty<float>("z");
      std::vector<float> r0 = plyIn.getElement("vertex").getProperty<float>("rot_0");
      std::vector<float> r1 = plyIn.getElement("vertex").getProperty<float>("rot_1");
      std::vector<float> r2 = plyIn.getElement("vertex").getProperty<float>("rot_2");
      std::vector<float> r3 = plyIn.getElement("vertex").getProperty<float>("rot_3");

      std::vector<float> npx, npy, npz, nr0, nr1, nr2, nr3;

      for (int i : existingIndices) {
        // Transform position
        glm::vec4 pos = splatModelMatrix * glm::vec4(px[i], py[i], pz[i], 1.0f);
        npx.push_back(pos.x);
        npy.push_back(pos.y);
        npz.push_back(pos.z);

        // Transform rotation (Quaternion multiplication)
        glm::quat oldRot(r0[i], r1[i], r2[i], r3[i]);
        glm::quat newRot = bakeRot * oldRot;
        nr0.push_back(newRot.w);
        nr1.push_back(newRot.x);
        nr2.push_back(newRot.y);
        nr3.push_back(newRot.z);
      }

      for (const auto& property : properties) {
        if (property == "x")
          plyOut.getElement(elementName).addProperty("x", npx);
        else if (property == "y")
          plyOut.getElement(elementName).addProperty("y", npy);
        else if (property == "z")
          plyOut.getElement(elementName).addProperty("z", npz);
        else if (property == "rot_0")
          plyOut.getElement(elementName).addProperty("rot_0", nr0);
        else if (property == "rot_1")
          plyOut.getElement(elementName).addProperty("rot_1", nr1);
        else if (property == "rot_2")
          plyOut.getElement(elementName).addProperty("rot_2", nr2);
        else if (property == "rot_3")
          plyOut.getElement(elementName).addProperty("rot_3", nr3);
        else {
          // Pass-through other properties
          auto data = plyIn.getElement(elementName).getProperty<float>(property);
          std::vector<float> filtered;
          for (int i : existingIndices) filtered.push_back(data[i]);
          plyOut.getElement(elementName).addProperty(property, filtered);
        }
      }
    } else {
      // Other elements (if any)
      for (const auto& property : properties) {
        auto data = plyIn.getElement(elementName).getProperty<float>(property);
        std::vector<float> filtered;
        for (int i : existingIndices) filtered.push_back(data[i]);
        plyOut.getElement(elementName).addProperty(property, filtered);
      }
    }
  }

  plyOut.write(outputPath, happly::DataFormat::Binary);
  std::cout << "Baking completed and saved to " << outputPath << std::endl;
}

void App::printInfo(happly::PLYData& plyIn) {
  auto comments = plyIn.comments;
  std::cout << "comments: " << std::endl;
  for (const auto& comment : comments) {
    std::cout << comment << std::endl;
  }
  auto infoComments = plyIn.objInfoComments;
  std::cout << "info comments: " << std::endl;
  for (const auto& infoComment : infoComments) {
    std::cout << infoComment << std::endl;
  }
  auto elementNames = plyIn.getElementNames();
  std::cout << "element names: " << std::endl;
  for (const auto& elementName : elementNames) {
    std::cout << "- " << elementName << std::endl;
    auto properties = plyIn.getElement(elementName).getPropertyNames();
    std::cout << "  properties: " << std::endl;
    for (const auto& property : properties) {
      std::cout << "  - " << property << std::endl;
    }
  }
}

void App::showPathSelector() {
  if (ImGui::Button("Select .ply File")) {
    IGFD::FileDialogConfig config;
    config.path = "./assets/";
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose .ply File", ".ply", config);
  }

  if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      modelPath = ImGuiFileDialog::Instance()->GetFilePathName();
      splat = loadPlyFile(modelPath);
      splat->sort(getViewModelMatrix());
      std::cout << "Done! " << std::endl;
    }

    ImGuiFileDialog::Instance()->Close();
  }
}
