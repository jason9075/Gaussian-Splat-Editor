#version 330 core

in vec3 Color;
in vec2 XYValue;
in vec4 con_o;
in float ScaleModif;

out vec4 FragColor;

void main()
{
    if (XYValue.x < 0.0 || XYValue.y < 0.0 || XYValue.x > 1024.0 || XYValue.y > 768.0){
        discard;
    }
    vec2 diff = XYValue - gl_FragCoord.xy;
    float power = -0.5 * (con_o.x * diff.x * diff.x + con_o.z * diff.y * diff.y) - con_o.y * diff.x * diff.y;

    power *= ScaleModif;

    if (power > 0.0){
        discard;
    }

    float alpha = min(.99f, con_o.w * exp(power)); // con_o.w is the opacity

    // remove squares corners
    if (alpha < 0.01f){
        discard;
    }

    FragColor = vec4(Color*alpha, alpha);
}
