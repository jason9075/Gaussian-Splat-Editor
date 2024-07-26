#version 330 core

layout (location = 0) in vec3 aPos;

uniform vec3 Size;
uniform vec3 Position;

uniform mat4 modelMatrix;
uniform mat4 camMatrix;

void main()
{
    vec3 Pos = aPos * Size + Position;
    gl_Position = camMatrix * vec4(Pos, 1.0);
}
