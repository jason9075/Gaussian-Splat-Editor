#include "App.h"

#include <OPPCH.h>

App::App(const float screenWidth, const float screenHeight) {
  glViewport(0, 0, screenWidth, screenHeight);

  shaderProgram =
      std::make_unique<Shader>("./shaders/geo_vert.glsl", "./shaders/geo_gert.glsl", "./shaders/geo_frag.glsl");

  happly::PLYData plyIn("./assets/Medic.ply");

  // Print Info
  // printInfo(plyIn);

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

  std::vector<GaussianSphere> spheres;
  for (int i = 0; i < x.size(); i++) {
    // for (int i = 0; i < 1000; i++) {
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
    spheres.push_back(sphere);
  }
  splat = std::make_unique<GaussianSplat>(spheres);

  glm::vec3 position = glm::vec3(5.0f, 3.0f, 0.0f);
  glm::vec3 orientation = glm::vec3(-0.7f, -0.6f, 0.0f);
  camera = std::make_unique<Camera>(screenWidth, screenHeight, position, orientation);
  listener = std::make_unique<GhostCameraListener>(camera.get());
  camera->setEventListener(listener.get());

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  // TODO: fix z value is opposite
  // if camera is looking at the origin, the z value should be negative
  splat->sort(camera->viewMatrix, false);
}
App::~App() {}

void App::OnEvent(SDL_Event& event) { camera->handle(event); }

void App::OnRender() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  camera->moveCamera();

  shaderProgram->use();
  glUniformMatrix4fv(
      glGetUniformLocation(shaderProgram->ID, "modelMatrix"), 1, GL_FALSE,
      glm::value_ptr(glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f)),
                                 glm::radians(rotateZ), glm::vec3(0.0f, 0.0f, 1.0f))));
  glUniform1f(glGetUniformLocation(shaderProgram->ID, "scaleFactor"), scaleFactor);
  glUniform1f(glGetUniformLocation(shaderProgram->ID, "W"), 1024.0f);
  glUniform1f(glGetUniformLocation(shaderProgram->ID, "H"), 768.0f);
  float tan_fovx = tan(glm::radians(45.0f) / 2.0f);
  float tan_fovy = tan(glm::radians(45.0f) / 2.0f);
  float focal_y = 768.0f / (2.0f * tan_fovy);
  float focal_x = 1024.0f / (2.0f * tan_fovx);
  glUniform1f(glGetUniformLocation(shaderProgram->ID, "focal_x"), focal_x);
  glUniform1f(glGetUniformLocation(shaderProgram->ID, "focal_y"), focal_y);
  glUniform1f(glGetUniformLocation(shaderProgram->ID, "tan_fovx"), tan_fovx);
  glUniform1f(glGetUniformLocation(shaderProgram->ID, "tan_fovy"), tan_fovy);
  camera->update(shaderProgram.get());
  // splat->sort(camera->viewMatrix, false);
  splat->draw(shaderProgram.get());
}

void App::OnImGuiRender() {
  ImGui::Text("Camera Position:");
  ImGui::Text("X:%.2f Y:%.2f Z:%.2f", camera->position.x, camera->position.y, camera->position.z);
  ImGui::Text("Camera Orientation:");
  ImGui::Text("X:%.2f Y:%.2f Z:%.2f", camera->orientation.x, camera->orientation.y, camera->orientation.z);
  ImGui::Text("View Matrix:");

  glm::mat4 vm = camera->viewMatrix;
  ImGui::Text("%.2f %.2f %.2f %.2f", vm[0][0], vm[0][1], vm[0][2], vm[0][3]);
  ImGui::Text("%.2f %.2f %.2f %.2f", vm[1][0], vm[1][1], vm[1][2], vm[1][3]);
  ImGui::Text("%.2f %.2f %.2f %.2f", vm[2][0], vm[2][1], vm[2][2], vm[2][3]);
  ImGui::Text("%.2f %.2f %.2f %.2f", vm[3][0], vm[3][1], vm[3][2], vm[3][3]);

  ImGui::SliderFloat("Rot-X", &rotateX, -180., 180.);
  ImGui::SliderFloat("Rot-Z", &rotateZ, -180., 180.);
  ImGui::SliderFloat("ScaleF", &scaleFactor, 0.1f, 3.0f);
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
