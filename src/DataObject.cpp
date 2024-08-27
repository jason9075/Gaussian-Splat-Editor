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

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices)
    : vertices(vertices), indices(indices), vao(), vbo(vertices), ebo() {
  vao.bind();
  ebo.bind();
  ebo.bufferData(indices);

  vao.linkAttr(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, position));
  vao.linkAttr(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, normal));
  vao.linkAttr(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, color));

  vao.unbind();
}

void Mesh::draw(Shader *shader) {
  vao.bind();

  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

  vao.unbind();
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

void GaussianSplat::rotateX(float degree) {
  glm::mat3 R(glm::rotate(glm::mat4(1.0f), glm::radians(degree), glm::vec3(1.0f, 0.0f, 0.0f)));
  for (size_t i = 0; i < spheres.size(); ++i) {
    glm::mat3 M =
        R *
        glm::mat3(spheres[i].covA.x, spheres[i].covA.y, spheres[i].covA.z, spheres[i].covA.y, spheres[i].covB.x,
                  spheres[i].covB.y, spheres[i].covA.z, spheres[i].covB.y, spheres[i].covB.z) *
        glm::transpose(R);
    spheres[i].position = R * spheres[i].position;
    spheres[i].covA = glm::vec3(M[0][0], M[0][1], M[0][2]);
    spheres[i].covB = glm::vec3(M[1][1], M[1][2], M[2][2]);
  }
  vbo.bind();
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GaussianSphere) * spheres.size(), spheres.data());
  vbo.unbind();
}

void GaussianSplat::rotateZ(float degree) {
  glm::mat3 R(glm::rotate(glm::mat4(1.0f), glm::radians(degree), glm::vec3(0.0f, 0.0f, 1.0f)));
  for (size_t i = 0; i < spheres.size(); ++i) {
    glm::mat3 M =
        R *
        glm::mat3(spheres[i].covA.x, spheres[i].covA.y, spheres[i].covA.z, spheres[i].covA.y, spheres[i].covB.x,
                  spheres[i].covB.y, spheres[i].covA.z, spheres[i].covB.y, spheres[i].covB.z) *
        glm::transpose(R);
    spheres[i].position = R * spheres[i].position;
    spheres[i].covA = glm::vec3(M[0][0], M[0][1], M[0][2]);
    spheres[i].covB = glm::vec3(M[1][1], M[1][2], M[2][2]);
  }
  vbo.bind();
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GaussianSphere) * spheres.size(), spheres.data());
  vbo.unbind();
}

void GaussianSplat::removeSplats(float position[3], float size[3]) {
  float minX = position[0] - size[0];
  float maxX = position[0] + size[0];
  float minY = position[1] - size[1];
  float maxY = position[1] + size[1];
  float minZ = position[2] - size[2];
  float maxZ = position[2] + size[2];
  auto it = std::remove_if(
      spheres.begin(), spheres.end(), [minX, maxX, minY, maxY, minZ, maxZ](const GaussianSphere &sphere) {
        return minX <= sphere.position.x && sphere.position.x <= maxX && minY <= sphere.position.y &&
               sphere.position.y <= maxY && minZ <= sphere.position.z && sphere.position.z <= maxZ;
      });
  spheres.erase(it, spheres.end());

  vbo.bind();
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GaussianSphere) * spheres.size(), spheres.data());
  vbo.unbind();
}

void GaussianSplat::draw(Shader *shader) {
  vao.bind();
  glDrawElements(GL_POINTS, spheres.size(), GL_UNSIGNED_INT, 0);
  vao.unbind();
}

std::vector<int> GaussianSplat::getExistingIndices() {
  std::vector<int> existingIndices;
  for (size_t i = 0; i < spheres.size(); ++i) {
    existingIndices.push_back(spheres[i].index);
  }
  return existingIndices;
}

void GaussianSplat::del() {
  vao.del();
  vbo.del();
}
