#include "DataObject.h"

#include <OPPCH.h>

#include <chrono>

#include "stb_image.h"

VAO::VAO() { glGenVertexArrays(1, &ID); }

void VAO::linkAttr(VBO &VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, const void *offset) {
  VBO.bind();
  glEnableVertexAttribArray(layout);
  glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
  VBO.unbind();
}

void VAO::bind() { glBindVertexArray(ID); }

void VAO::unbind() { glBindVertexArray(0); }

void VAO::del() { glDeleteVertexArrays(1, &ID); }

template <typename T>
VBO::VBO(const std::vector<T> &data) {
  glGenBuffers(1, &ID);
  glBindBuffer(GL_ARRAY_BUFFER, ID);
  glBufferData(GL_ARRAY_BUFFER, sizeof(T) * data.size(), data.data(), GL_STATIC_DRAW);
}

void VBO::bind() { glBindBuffer(GL_ARRAY_BUFFER, ID); }

void VBO::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

void VBO::del() { glDeleteBuffers(1, &ID); }

// note: EBO no need to unbind
EBO::EBO() { glGenBuffers(1, &ID); }

void EBO::bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); }

void EBO::bufferData(const std::vector<GLuint> &indices) {
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
}

void EBO::del() { glDeleteBuffers(1, &ID); }

Texture::Texture(const char *image, const char *texType, GLuint slot) : type(texType) {
  int width, height, nrChannels;

  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(image, &width, &height, &nrChannels, 0);

  glGenTextures(1, &ID);
  glActiveTexture(GL_TEXTURE0 + slot);
  unit = slot;
  glBindTexture(GL_TEXTURE_2D, ID);

  if (data) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    std::cerr << "Failed to load texture: " << image << std::endl;
  }

  if (nrChannels == 4) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  } else if (nrChannels == 3) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  } else if (nrChannels == 1) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
  } else {
    throw std::invalid_argument("Automatic conversion of image channels is not supported.");
  }

  // Generate mipmaps
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::texUnit(Shader &shader, const char *uniform, GLuint unit) {
  glUniform1i(glGetUniformLocation(shader.ID, uniform), unit);
}

void Texture::bind() {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

void Texture::del() { glDeleteTextures(1, &ID); }

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices,
           const std::vector<Texture> &textures)
    : vertices(vertices), indices(indices), textures(textures), vao(), vbo(vertices), ebo() {
  vao.bind();
  ebo.bind();
  ebo.bufferData(indices);

  vao.linkAttr(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, position));
  vao.linkAttr(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, normal));
  vao.linkAttr(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, color));
  vao.linkAttr(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, texCoords));

  vao.unbind();
}

void Mesh::draw(Shader *shader) {
  vao.bind();

  // Textures
  for (unsigned int i = 0; i < textures.size(); i++) {
    textures[i].texUnit(*shader, "texture0", i);
    textures[i].bind();
  }

  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

  vao.unbind();
  for (unsigned int i = 0; i < textures.size(); i++) {
    textures[i].unbind();
  }
}

void Mesh::del() {
  vao.del();
  vbo.del();
  ebo.del();
}

GaussianSplat::GaussianSplat(const std::vector<GaussianSphere> &spheres)
    : vao(), vbo(spheres), ebo(), spheres(spheres) {
  std::vector<GLuint> indices(spheres.size());
  vao.bind();
  ebo.bind();
  ebo.bufferData(indices);

  vao.linkAttr(vbo, 0, 3, GL_FLOAT, sizeof(GaussianSphere), (void *)offsetof(GaussianSphere, position));
  vao.linkAttr(vbo, 1, 3, GL_FLOAT, sizeof(GaussianSphere), (void *)offsetof(GaussianSphere, color));
  vao.linkAttr(vbo, 2, 1, GL_FLOAT, sizeof(GaussianSphere), (void *)offsetof(GaussianSphere, opacity));
  vao.linkAttr(vbo, 3, 3, GL_FLOAT, sizeof(GaussianSphere), (void *)offsetof(GaussianSphere, covA));
  vao.linkAttr(vbo, 4, 3, GL_FLOAT, sizeof(GaussianSphere), (void *)offsetof(GaussianSphere, covB));

  vao.unbind();
}

void GaussianSplat::sort(const glm::mat4 &vmMatrix) {
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<std::pair<size_t, float>> zValues(spheres.size());

  for (size_t i = 0; i < spheres.size(); ++i) {
    float z = spheres[i].position.x * vmMatrix[0][2] + spheres[i].position.y * vmMatrix[1][2] +
              spheres[i].position.z * vmMatrix[2][2] + vmMatrix[3][2];
    zValues[i] = std::make_pair(i, z);
  }
  std::sort(zValues.begin(), zValues.end(),
            [](const std::pair<size_t, float> &a, const std::pair<size_t, float> &b) { return a.second < b.second; });

  // Update the EBO
  std::vector<GLuint> indices(spheres.size());
  for (size_t i = 0; i < zValues.size(); ++i) {
    indices[i] = zValues[i].first;
  }

  vao.bind();
  ebo.bind();
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * indices.size(), indices.data());
  vao.unbind();
  std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start;
  std::cout << "Sort time: " << elapsed.count() << "s" << std::endl;
}

void GaussianSplat::draw(Shader *shader) {
  vao.bind();
  glDrawElements(GL_POINTS, spheres.size(), GL_UNSIGNED_INT, 0);
  vao.unbind();
}

void GaussianSplat::del() {
  vao.del();
  vbo.del();
}
