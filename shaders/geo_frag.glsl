#version 330 core

in vec2 CenterPix; // Gaussian center in pixels
in vec3 Cov; // 2D Gaussian covariance values
in vec3 Color;
in float Opacity;
in float ScaleModif;

uniform vec2 Resolution;

out vec4 FragColor;

void main()
{
    vec2 diff = CenterPix - gl_FragCoord.xy;
    float power = -0.5 * (Cov.x * diff.x * diff.x + Cov.z * diff.y * diff.y) - Cov.y * diff.x * diff.y;

    power *= ScaleModif; // if ScaleModif is greater, the final alpha will be smaller

    if (power > 0.0){
        discard;
    }

    float alpha = min(.99f, Opacity * exp(power));

    // remove squares corners
    if (alpha < 0.01f){
        discard;
    }

    FragColor = vec4(Color*alpha, alpha);
}
