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
  float rotateX = 35.;
  float rotateZ = 180.;
  const float C0 = 0.28209479f;
  float scaleFactor = 1.0f;
  std::unique_ptr<GaussianSplat> splat;
  std::unique_ptr<Shader> shaderProgram;
  std::unique_ptr<Camera> camera;
  std::unique_ptr<CameraEventListener> listener;
  void printInfo(happly::PLYData& plyIn);
};
