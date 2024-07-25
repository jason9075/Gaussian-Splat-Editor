#pragma once

#include "DataObject.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Model {
 public:
  Model(const char *path);

  void draw(Shader *shader);

  void setModelMatrix(glm::mat4 matrix);

 private:
  const char *path;
  json JSON;
  std::vector<unsigned char> data;  // binary data

  std::vector<Mesh> meshes;  // one model can have multiple meshes
  std::vector<std::string> loadedTexturesName;
  std::vector<Texture> loadedTextures;

  glm::mat4 modelMatrix;

  void loadMesh(unsigned int indMesh, glm::mat4 matrix);

  void traverseNode(unsigned int nodeIndex, glm::mat4 identity = glm::mat4(1.0f));

  std::vector<unsigned char> getData();

  std::vector<float> getFloats(json accessor);
  std::vector<GLuint> getIndices(json accessor);
  std::vector<Texture> getTextures();

  std::vector<Vertex> assembleVertices(const std::vector<glm::vec3> &positions, const std::vector<glm::vec3> &normals,
                                       const std::vector<glm::vec2> &texCoords);

  std::vector<glm::vec2> groupFloatsVec2(const std::vector<float> &floats);
  std::vector<glm::vec3> groupFloatsVec3(const std::vector<float> &floats);
  std::vector<glm::vec4> groupFloatsVec4(const std::vector<float> &floats);
};
