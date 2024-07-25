#pragma once

class Shader {
 public:
  Shader(const char *vertexShaderSource, const char *fragmentShaderSource);
  Shader(const char *vertexShaderSource, const char *geometryShaderSource, const char *fragmentShaderSource);
  GLuint ID;

  void use();
  void del();

 protected:
  std::string readFile(const char *filePath);

  GLuint compileShader(const char *shaderSource, GLenum shaderType);

  void checkCompileErrors(GLuint shader, std::string type);

  void checkLinkErrors(GLuint shader, std::string type);

 private:
};
