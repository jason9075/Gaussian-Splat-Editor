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
  glm::vec2 texCoords;
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

class Texture {
 public:
  const char *type;
  Texture(const char *image, const char *texType, GLuint slot);
  void texUnit(Shader &shader, const char *uniform, GLuint unit);
  void bind();
  void unbind();
  void del();

 private:
  GLuint ID;
  GLuint unit;
};

class Mesh {
 public:
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  std::vector<Texture> textures;

  VAO vao;
  VBO vbo;
  EBO ebo;

  Mesh(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices, const std::vector<Texture> &textures);

  void draw(Shader *shader);

  void del();
};

class GaussianSplat {
 public:
  VAO vao;
  VBO vbo;
  std::vector<GaussianSphere> spheres;

  GaussianSplat(const std::vector<GaussianSphere> &spheres);

  void sort(const glm::mat4 &viewMatrix, const bool isAscending = true);
  void draw(Shader *shader);

  void del();
};
