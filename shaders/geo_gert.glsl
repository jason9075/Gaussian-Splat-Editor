#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec2 GeoCenterPix[];
in float GeoRadius[]; 
in vec3 GeoCov[];
in vec3 GeoColor[];
in float GeoOpacity[];
in float GeoScaleModif[];
flat in int GeoIsSelected[];

uniform mat4 camMatrix;
uniform vec2 Resolution;

out vec2 CenterPix;
out vec3 Cov;
out vec3 Color;
out float Opacity;
out float ScaleModif;
flat out int IsSelected;

void main()
{
    // Pass through to fragment shader
    CenterPix = GeoCenterPix[0];
    Color = GeoColor[0];
    Opacity = GeoOpacity[0];
    Cov = GeoCov[0];
    ScaleModif = GeoScaleModif[0];
    IsSelected = GeoIsSelected[0];

    // Top left, bottom left, top right, bottom right
    vec2 offsets[4] = vec2[4](vec2(-1, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1));

    for (int i = 0; i < 4; i++)
    {
        vec2 screen_pos = CenterPix + GeoRadius[0] * offsets[i];
        vec2 clipPos = screen_pos / Resolution * 2.0 - 1.0;
        gl_Position = vec4(clipPos, 0.0, 1.0);
        EmitVertex();
    }

    EndPrimitive();
}
