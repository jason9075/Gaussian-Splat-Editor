#pragma once

#include "ShaderClass.h"

class VBO;

class VAO {
 public:
  VAO();
  void linkAttr(VBO &VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, const void *offset);
  void linkAttrDiv(VBO &VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, const void *offset);
  void bind();
  void unbind();
  void del();
  GLuint ID;

 private:
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 color;
};

struct Point {
  glm::vec3 position;
  glm::vec3 color;
  glm::vec3 scale;
  glm::vec4 rotation;
};

struct GaussianSphere {
  glm::vec3 position;
  glm::vec3 color;
  float opacity;
  glm::vec3 covA;
  glm::vec3 covB;
  int index;
};

class VBO {
 public:
  template <typename T>
  VBO(const std::vector<T> &data);
  void bind();
  void unbind();
  void del();

 private:
  GLuint ID;
};

class EBO {
 public:
  EBO();
  void bind();
  void bufferData(const std::vector<GLuint> &indices);
  void del();

 private:
  GLuint ID;
};

class Mesh {
 public:
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  VAO vao;
  VBO vbo;
  EBO ebo;

  Mesh(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices);

  void draw(Shader *shader);

  void del();
};

class GaussianSplat {
 public:
  VAO vao;
  VBO vbo;
  EBO ebo;
  std::vector<GaussianSphere> spheres;

  GaussianSplat(const std::vector<GaussianSphere> &spheres);

  void rotateX(float angle);
  void rotateZ(float angle);
  void removeSplats(float boxPosition[3], float boxSize[3]);
  void sort(const glm::mat4 &vmMatrix);
  void draw(Shader *shader);
  std::vector<int> getExistingIndices();

  void del();
};
