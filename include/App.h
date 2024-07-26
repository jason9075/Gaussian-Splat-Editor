#pragma once

#include "Camera.h"
#include "DataObject.h"
#include "ShaderClass.h"
#include "happly.h"

class App {
 public:
  App(const float screenWidth, const float screenHeight);
  ~App();

  void OnEvent(SDL_Event& event);
  void OnRender();
  void OnImGuiRender();

 private:
  float width;
  float height;
  float tan_fovx;
  float tan_fovy;
  float focal_x;
  float focal_y;
  const float C0 = 0.28209479f;
  float scaleFactor = 1.0f;
  std::unique_ptr<Mesh> boxMesh;
  float boxSize[3] = {1.0f, 1.0f, 1.0f};
  float boxPosition[3] = {0.0f, 0.0f, 0.0f};
  std::unique_ptr<GaussianSplat> splat;
  std::unique_ptr<Shader> boxShader;
  std::unique_ptr<Shader> splatShader;
  std::unique_ptr<Camera> camera;
  std::unique_ptr<CameraEventListener> listener;
  glm::mat4 getViewModelMatrix();
  void printInfo(happly::PLYData& plyIn);
};
