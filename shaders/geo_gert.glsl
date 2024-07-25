#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 GeoColor[];
in vec2 GeoPointImage[];
in float GeoRadius[]; 
in float GeoScaleModif[];
in vec4 Geocon_o[];

uniform mat4 camMatrix;

out vec3 Color;
out vec2 Pixf;
out float ScaleModif;
out vec4 con_o;
out vec2 XYValue;

void main()
{
    float W = 1024;
    float H = 768;

    Color = GeoColor[0];
    con_o = Geocon_o[0];
    XYValue = GeoPointImage[0];
    ScaleModif = GeoScaleModif[0];

    // 左下角
    vec2 screen_pos = vec2(GeoPointImage[0].x, GeoPointImage[0].y) + GeoRadius[0] * vec2(-1, -1);
    Pixf = screen_pos;
    vec2 clipPos = screen_pos / vec2(W, H) * 2.0 - 1.0;
    gl_Position = vec4(clipPos, 0.0, 1.0);
    EmitVertex();

    // 右下角
    screen_pos = vec2(GeoPointImage[0].x, GeoPointImage[0].y) + GeoRadius[0] * vec2(1, -1);
    Pixf = screen_pos;
    clipPos = screen_pos / vec2(W, H) * 2.0 - 1.0;
    gl_Position = vec4(clipPos, 0.0, 1.0);
    EmitVertex();

    // 左上角
    screen_pos = vec2(GeoPointImage[0].x, GeoPointImage[0].y) + GeoRadius[0] * vec2(-1, 1);
    Pixf = screen_pos;
    clipPos = screen_pos / vec2(W, H) * 2.0 - 1.0;
    gl_Position = vec4(clipPos, 0.0, 1.0);
    EmitVertex();

    // 右上角
    screen_pos = vec2(GeoPointImage[0].x, GeoPointImage[0].y) + GeoRadius[0] * vec2(1, 1);
    Pixf = screen_pos;
    clipPos = screen_pos / vec2(W, H) * 2.0 - 1.0;
    gl_Position = vec4(clipPos, 0.0, 1.0);
    EmitVertex();

    EndPrimitive();
}
