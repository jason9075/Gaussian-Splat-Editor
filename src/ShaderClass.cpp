#include "ShaderClass.h"

#include <OPPCH.h>

Shader::Shader(const char *vertShaderPath, const char *fragShaderPath) {
  std::string vertexCode = readFile(vertShaderPath);
  std::string fragmentCode = readFile(fragShaderPath);

  const char *vertexShaderSource = vertexCode.c_str();
  const char *fragmentShaderSource = fragmentCode.c_str();

  // Build and compile our shader program
  // Vertex shader
  GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
  checkCompileErrors(vertexShader, "VERTEX");
  // Fragment shader
  GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
  checkCompileErrors(fragmentShader, "FRAGMENT");
  // Link shaders
  ID = glCreateProgram();
  glAttachShader(ID, vertexShader);
  glAttachShader(ID, fragmentShader);
  glLinkProgram(ID);
  checkLinkErrors(ID, "PROGRAM");
  // Delete the shaders as they're linked into our program now and no longer
  // necessary
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

Shader::Shader(const char *vertShaderPath, const char *gertShaderPath, const char *fragShaderPath) {
  std::string vertexCode = readFile(vertShaderPath);
  std::string geometryCode = readFile(gertShaderPath);
  std::string fragmentCode = readFile(fragShaderPath);

  const char *vertexShaderSource = vertexCode.c_str();
  const char *geometryShaderSource = geometryCode.c_str();
  const char *fragmentShaderSource = fragmentCode.c_str();

  // Build and compile our shader program
  // Vertex shader
  GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
  checkCompileErrors(vertexShader, "VERTEX");
  // Geometry shader
  GLuint geometryShader = compileShader(geometryShaderSource, GL_GEOMETRY_SHADER);
  checkCompileErrors(geometryShader, "GEOMETRY");
  // Fragment shader
  GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
  checkCompileErrors(fragmentShader, "FRAGMENT");
  // Link shaders
  ID = glCreateProgram();
  glAttachShader(ID, vertexShader);
  glAttachShader(ID, geometryShader);
  glAttachShader(ID, fragmentShader);
  glLinkProgram(ID);
  checkLinkErrors(ID, "PROGRAM");
  // Delete the shaders as they're linked into our program now and no longer
  // necessary
  glDeleteShader(vertexShader);
  glDeleteShader(geometryShader);
  glDeleteShader(fragmentShader);
}

void Shader::use() { glUseProgram(ID); }

void Shader::del() { glDeleteProgram(ID); }

std::string Shader::readFile(const char *filePath) {
  std::string content;
  std::ifstream fileStream(filePath, std::ios::in);
  std::string line = "";
  while (!fileStream.eof()) {
    std::getline(fileStream, line);
    content.append(line + "\n");
  }
  fileStream.close();
  return content;
}

GLuint Shader::compileShader(const char *shaderSource, GLenum shaderType) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &shaderSource, NULL);
  glCompileShader(shader);
  return shader;
}

void Shader::checkCompileErrors(GLuint shader, std::string type) {
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
}

void Shader::checkLinkErrors(GLuint shader, std::string type) {
  GLint success;
  GLchar infoLog[512];
  glGetProgramiv(shader, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::" << type << "::LINKING_FAILED\n" << infoLog << std::endl;
  }
}
