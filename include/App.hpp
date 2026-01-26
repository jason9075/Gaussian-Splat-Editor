#pragma once

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Camera.hpp"
#include "DataObject.hpp"
#include "ShaderClass.hpp"
#include "happly.h"

class App {
 public:
  App(const float screenWidth, const float screenHeight);
  ~App();

  void OnEvent(SDL_Event& event);
  void OnRender();
  void OnImGuiRender();

 private:
  std::string modelPath;
  float width;
  float height;
  float tan_fovx;
  float tan_fovy;
  float focal_x;
  float focal_y;
  const float C0 = 0.28209479f;
  float scaleFactor = 1.0f;

  // Rotation Gizmo & Model Transform
  glm::mat4 splatModelMatrix = glm::mat4(1.0f);
  bool isRotating = false;
  glm::vec2 lastMousePos;
  const float gizmoSize = 150.0f;

  std::unique_ptr<Mesh> boxMesh;
  std::unique_ptr<Mesh> sphereMesh;
  std::unique_ptr<Mesh> coneMesh;

  // Gizmo Interaction State
  enum class GizmoMode {
    NONE,
    TRANSLATE_X,
    TRANSLATE_Y,
    TRANSLATE_Z,
    SCALE_PX,
    SCALE_NX,
    SCALE_PY,
    SCALE_NY,
    SCALE_PZ,
    SCALE_NZ
  };
  GizmoMode currentGizmoMode = GizmoMode::NONE;
  glm::vec3 dragStartWorldPos;
  glm::vec3 dragStartBoxPos;
  glm::vec3 dragStartBoxSize;

  glm::vec3 getMouseRay(float x, float y);
  bool intersectRaySphere(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& sphereCenter,
                          float radius, float& t);

  glm::vec3 getArcballVector(float x, float y);

  bool showSelectionBox = true;
  float boxSize[3] = {1.0f, 1.0f, 1.0f};
  float boxPosition[3] = {0.0f, 0.0f, 0.0f};
  std::unique_ptr<GaussianSplat> splat;
  std::unique_ptr<Shader> boxShader;
  std::unique_ptr<Shader> splatShader;
  std::unique_ptr<Camera> camera;
  std::unique_ptr<CameraEventListener> listener;
  glm::mat4 getViewModelMatrix();
  std::unique_ptr<GaussianSplat> loadPlyFile(const std::string path);
  void savePlyFile(const std::string path);
  void printInfo(happly::PLYData& plyIn);
  void showPathSelector();
};
