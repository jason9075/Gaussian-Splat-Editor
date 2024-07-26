#include "App.h"

#include <OPPCH.h>

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
  listener->onKeyUpCallback = [&](SDL_Keycode key) { splat->sort(getViewModelMatrix()); };
  listener->onMouseLeftReleaseCallback = [&]() { splat->sort(getViewModelMatrix()); };
  camera->setEventListener(listener.get());
  splat->sort(getViewModelMatrix());

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  // Box Mesh
  boxShader = std::make_unique<Shader>("./shaders/box_vert.glsl", "./shaders/box_frag.glsl");
  std::vector<Vertex> boxVertices = {
      {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}}, {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
      {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}},   {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
      {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},  {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
      {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},    {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
  };
  std::vector<GLuint> boxIndices = {
      0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7, 4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3,
  };
  boxMesh = std::make_unique<Mesh>(boxVertices, boxIndices);
}
App::~App() {}

void App::OnEvent(SDL_Event& event) { camera->handle(event); }

void App::OnRender() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  camera->moveCamera();

  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  splatShader->use();
  glUniformMatrix4fv(glGetUniformLocation(splatShader->ID, "modelMatrix"), 1, GL_FALSE,
                     glm::value_ptr(glm::mat4(1.0f)));
  glUniform1f(glGetUniformLocation(splatShader->ID, "scaleFactor"), scaleFactor);
  glUniform2f(glGetUniformLocation(splatShader->ID, "Resolution"), width, height);
  glUniform2f(glGetUniformLocation(splatShader->ID, "Focal"), focal_x, focal_y);
  glUniform2f(glGetUniformLocation(splatShader->ID, "TanFov"), tan_fovx, tan_fovy);
  // calc box bounds location
  glm::vec3 boxMin = glm::vec3(boxPosition[0] - boxSize[0], boxPosition[1] - boxSize[1], boxPosition[2] - boxSize[2]);
  glm::vec3 boxMax = glm::vec3(boxPosition[0] + boxSize[0], boxPosition[1] + boxSize[1], boxPosition[2] + boxSize[2]);
  glUniform3fv(glGetUniformLocation(splatShader->ID, "BoxMin"), 1, glm::value_ptr(boxMin));
  glUniform3fv(glGetUniformLocation(splatShader->ID, "BoxMax"), 1, glm::value_ptr(boxMax));
  camera->update(splatShader.get());
  splat->draw(splatShader.get());

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  boxShader->use();
  camera->update(boxShader.get());
  glUniform3fv(glGetUniformLocation(boxShader->ID, "Size"), 1, boxSize);
  glUniform3fv(glGetUniformLocation(boxShader->ID, "Position"), 1, boxPosition);
  boxMesh->draw(boxShader.get());
}

void App::OnImGuiRender() {
  ImGui::Text("Camera Position:");
  ImGui::Text("X:%.2f Y:%.2f Z:%.2f", camera->position.x, camera->position.y, camera->position.z);
  ImGui::Text("Camera Orientation:");
  ImGui::Text("X:%.2f Y:%.2f Z:%.2f", camera->orientation.x, camera->orientation.y, camera->orientation.z);

  if (ImGui::Button("Rot-X +10 degrees")) {
    splat->rotateX(10);
  }
  if (ImGui::Button("Rot-X -10 degrees")) {
    splat->rotateX(-10);
  }
  if (ImGui::Button("Rot-Z +10 degrees")) {
    splat->rotateZ(10);
  }
  if (ImGui::Button("Rot-Z -10 degrees")) {
    splat->rotateZ(-10);
  }
  if (ImGui::Button("Remove Splat")) {
    splat->removeSplats(boxPosition, boxSize);
    splat->sort(getViewModelMatrix());
  }
  ImGui::SliderFloat("ScaleF", &scaleFactor, 0.1f, 3.0f);

  ImGui::Text("Box:");
  ImGui::SliderFloat3("Scale", boxSize, 0.1f, 20.0f);
  ImGui::SliderFloat3("Position", boxPosition, -50.0f, 50.0f);
}

glm::mat4 App::getViewModelMatrix() { return camera->viewMatrix; }

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
