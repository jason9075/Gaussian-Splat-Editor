#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aScale;
layout(location = 3) in vec4 aRot;

out vec3 color;

uniform mat4 modelMatrix;
uniform mat4 camMatrix;


void main() {
    gl_Position = camMatrix * modelMatrix * vec4(aPos, 1.0);
    color = aColor;


    // change point size by distance

    // vec3 scalePos = aPos * aScale;
    // gl_Position = camMatrix * modelMatrix * vec4(scalePos, 1.0);
    // // set point size by camera distance
    // float distance = length(gl_Position.xyz);
    // float size = exp(-distance * 0.3) * 5.0;
    // gl_PointSize = size;
}
