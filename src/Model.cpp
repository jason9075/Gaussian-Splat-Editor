
#include "Model.h"

#include <OPPCH.h>

#include "Utils.h"

Model::Model(const char *path) {
  modelMatrix = glm::mat4(1.0f);
  std::string text = readFile(path);
  JSON = json::parse(text);

  Model::path = path;
  data = getData();

  glm::mat4 identity = glm::mat4(1.0f);
  traverseNode(0, identity);

  // release data
  data.clear();
}

void Model::loadMesh(unsigned int indMesh, glm::mat4 matrix) {
  json primitive = JSON["meshes"][indMesh]["primitives"][0];

  unsigned int posAccInd = primitive["attributes"]["POSITION"];
  unsigned int norAccInd = primitive["attributes"]["NORMAL"];
  unsigned int texAccInd = primitive["attributes"]["TEXCOORD_0"];
  unsigned int indAccInd = primitive["indices"];

  std::vector<glm::vec3> positions = groupFloatsVec3(getFloats(JSON["accessors"][posAccInd]));
  std::vector<glm::vec3> normals = groupFloatsVec3(getFloats(JSON["accessors"][norAccInd]));
  std::vector<glm::vec2> texCoords = groupFloatsVec2(getFloats(JSON["accessors"][texAccInd]));

  // apply matrix to positions and normals
  for (unsigned int i = 0; i < positions.size(); i++) {
    glm::vec4 pos = matrix * glm::vec4(positions[i], 1.0f);
    positions[i] = glm::vec3(pos);
  }

  for (unsigned int i = 0; i < normals.size(); i++) {
    glm::vec4 nor = matrix * glm::vec4(normals[i], 0.0f);
    normals[i] = glm::vec3(nor);
  }

  std::vector<Vertex> vertices = assembleVertices(positions, normals, texCoords);
  std::vector<GLuint> indices = getIndices(JSON["accessors"][indAccInd]);
  std::vector<Texture> textures = getTextures();

  meshes.push_back(Mesh(vertices, indices, textures));
}

void Model::traverseNode(unsigned int nodeIndex, glm::mat4 modelMatrix) {
  json node = JSON["nodes"][nodeIndex];

  // Get model TRS or matrix (either TRS or matrix is present in the node)
  glm::mat4 nextModelMatrix = modelMatrix;
  bool hasMatrix = (node.find("matrix") != node.end());

  if (hasMatrix) {
    float mat[16];
    for (unsigned int i = 0; i < node["matrix"].size(); i++) {
      mat[i] = node["matrix"][i];
    }
    glm::mat4 matNode = glm::make_mat4(mat);
    nextModelMatrix = modelMatrix * matNode;
  } else {
    if (node.find("translation") != node.end()) {
      float transValues[3];
      for (unsigned int i = 0; i < node["translation"].size(); i++) {
        transValues[i] = node["translation"][i];
      }
      nextModelMatrix = glm::translate(nextModelMatrix, glm::make_vec3(transValues));
    }

    // gltf -> glm, (x, y, z, w) -> (w, x, y, z)
    if (node.find("rotation") != node.end()) {
      float rotValues[4] = {
          node["rotation"][3],
          node["rotation"][0],
          node["rotation"][1],
          node["rotation"][2],
      };
      nextModelMatrix = nextModelMatrix * glm::mat4_cast(glm::make_quat(rotValues));
    }

    if (node.find("scale") != node.end()) {
      float scaleValues[3];
      for (unsigned int i = 0; i < node["scale"].size(); i++) {
        scaleValues[i] = node["scale"][i];
      }
      nextModelMatrix = glm::scale(nextModelMatrix, glm::make_vec3(scaleValues));
    }
  }

  if (node.find("mesh") != node.end()) {
    loadMesh(node["mesh"], nextModelMatrix);
  }

  // keep traversing
  if (node.find("children") != node.end()) {
    for (unsigned int i = 0; i < node["children"].size(); i++) {
      traverseNode(node["children"][i], nextModelMatrix);
    }
  }
}

std::vector<unsigned char> Model::getData() {
  std::string bytesText;
  std::string uri = JSON["buffers"][0]["uri"];

  std::string fileStr = std::string(path);
  std::string fileDir = fileStr.substr(0, fileStr.find_last_of("/"));
  bytesText = readFile((fileDir + "/" + uri).c_str());

  std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
  return data;
}

std::vector<float> Model::getFloats(json accessor) {
  std::vector<float> floats;

  unsigned int bufferViewIndex = accessor.value("bufferView", 1);
  unsigned int count = accessor["count"];
  unsigned int accByteOffset = accessor.value("byteOffset", 0);
  std::string accType = accessor["type"];

  json bufferView = JSON["bufferViews"][bufferViewIndex];
  unsigned int byteOffset = bufferView.value("byteOffset", 0);

  unsigned int numPerVert;
  if (accType == "SCALAR")
    numPerVert = 1;
  else if (accType == "VEC2")
    numPerVert = 2;
  else if (accType == "VEC3")
    numPerVert = 3;
  else if (accType == "VEC4")
    numPerVert = 4;
  else
    throw std::runtime_error("Unknown type: " + accType);

  unsigned int beginningOfData = byteOffset + accByteOffset;
  unsigned int lengthOfData = count * numPerVert * 4;

  for (unsigned int i = beginningOfData; i < beginningOfData + lengthOfData; i) {
    unsigned char bytes[] = {data[i], data[i + 1], data[i + 2], data[i + 3]};
    float value;
    std::memcpy(&value, bytes, sizeof(float));
    floats.push_back(value);
    i += 4;
  }

  return floats;
}

std::vector<GLuint> Model::getIndices(json accessor) {
  std::vector<GLuint> indices;

  unsigned int bufferViewIndex = accessor.value("bufferView", 1);
  unsigned int count = accessor["count"];
  unsigned int accByteOffset = accessor.value("byteOffset", 0);
  unsigned int componentType = accessor["componentType"];

  json bufferView = JSON["bufferViews"][bufferViewIndex];
  unsigned int byteOffset = bufferView.value("byteOffset", 0);

  unsigned int beginningOfData = byteOffset + accByteOffset;
  if (componentType == 5122) {
    for (unsigned int i = beginningOfData; i < beginningOfData + count * 2; i += 2) {
      unsigned char bytes[] = {data[i], data[i + 1]};
      short value;
      std::memcpy(&value, bytes, sizeof(short));
      indices.push_back((GLuint)value);
    }
  } else if (componentType == 5123) {
    for (unsigned int i = beginningOfData; i < beginningOfData + count * 2; i += 2) {
      unsigned char bytes[] = {data[i], data[i + 1]};
      unsigned short value;
      std::memcpy(&value, bytes, sizeof(unsigned short));
      indices.push_back((GLuint)value);
    }
  } else if (componentType == 5125) {
    for (unsigned int i = beginningOfData; i < beginningOfData + count * 4; i += 4) {
      unsigned char bytes[] = {data[i], data[i + 1], data[i + 2], data[i + 3]};
      unsigned int value;
      std::memcpy(&value, bytes, sizeof(unsigned int));
      indices.push_back((GLuint)value);
    }
  } else {
    throw std::runtime_error("Unknown component type: " + std::to_string(componentType));
  }

  return indices;
}

std::vector<Texture> Model::getTextures() {
  std::vector<Texture> textures;

  std::string fileStr = std::string(path);
  std::string fileDir = fileStr.substr(0, fileStr.find_last_of("/"));

  for (unsigned int i = 0; i < JSON["images"].size(); i++) {
    std::string texPath = JSON["images"][i]["uri"];
    bool skip = false;
    for (unsigned int j = 0; j < loadedTexturesName.size(); j++) {
      if (loadedTexturesName[j] == texPath) {
        textures.push_back(loadedTextures[j]);
        skip = true;
        break;
      }
    }

    if (skip) continue;

    Texture tx = Texture((fileDir + "/" + texPath).c_str(), "normal", loadedTextures.size());
    textures.push_back(tx);
    loadedTextures.push_back(tx);
    loadedTexturesName.push_back(texPath);
  }

  return textures;
}

std::vector<Vertex> Model::assembleVertices(const std::vector<glm::vec3> &positions,
                                            const std::vector<glm::vec3> &normals,
                                            const std::vector<glm::vec2> &texCoords) {
  std::vector<Vertex> vertices;

  for (unsigned int i = 0; i < positions.size(); i++) {
    Vertex vertex;
    vertex.position = positions[i];
    vertex.normal = normals[i];
    vertex.color = glm::vec3(1.0f);
    vertex.texCoords = texCoords[i];
    vertices.push_back(vertex);
  }

  return vertices;
}

void Model::setModelMatrix(glm::mat4 matrix) { modelMatrix = matrix; }

void Model::draw(Shader *shader) {
  glUniformMatrix4fv(glGetUniformLocation(shader->ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
  for (unsigned int i = 0; i < meshes.size(); i++) {
    meshes[i].draw(shader);
  }
}

std::vector<glm::vec2> Model::groupFloatsVec2(const std::vector<float> &floats) {
  std::vector<glm::vec2> vec2s;

  for (unsigned int i = 0; i < floats.size(); i += 2) {
    vec2s.push_back(glm::vec2(floats[i], floats[i + 1]));
  }

  return vec2s;
}

std::vector<glm::vec3> Model::groupFloatsVec3(const std::vector<float> &floats) {
  std::vector<glm::vec3> vec3s;

  for (unsigned int i = 0; i < floats.size(); i += 3) {
    vec3s.push_back(glm::vec3(floats[i], floats[i + 1], floats[i + 2]));
  }

  return vec3s;
}

std::vector<glm::vec4> Model::groupFloatsVec4(const std::vector<float> &floats) {
  std::vector<glm::vec4> vec4s;

  for (unsigned int i = 0; i < floats.size(); i += 4) {
    vec4s.push_back(glm::vec4(floats[i], floats[i + 1], floats[i + 2], floats[i + 3]));
  }

  return vec4s;
}
