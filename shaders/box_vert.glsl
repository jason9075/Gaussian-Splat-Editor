#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform vec3 Size;
uniform vec3 Position;

uniform mat4 modelMatrix;
uniform mat4 camMatrix;

out vec3 Normal;

void main()
{
    vec3 Pos = aPos * Size + Position;
    Normal = mat3(transpose(inverse(modelMatrix))) * aNormal;
    gl_Position = camMatrix * modelMatrix * vec4(Pos, 1.0);
}
