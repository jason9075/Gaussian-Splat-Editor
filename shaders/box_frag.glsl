#version 330 core

uniform vec3 Color = vec3(1.0, 1.0, 1.0);

in vec3 Normal;
out vec4 fragColor;

void main()
{
   vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
   float diff = max(dot(normalize(Normal), lightDir), 0.0);
   vec3 diffuse = (diff * 0.7 + 0.3) * Color;
   fragColor = vec4(diffuse, 0.5);
}
