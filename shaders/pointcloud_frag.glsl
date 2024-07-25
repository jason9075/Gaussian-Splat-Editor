#version 330 core

in vec3 color;

out vec4 fragColor;

void main() {
    fragColor = vec4(color, 1);

    // convert square to circle

    // vec2 coord = gl_PointCoord - vec2(0.5);
    // if (dot(coord, coord) > 0.25) {
    //     discard;  // Discard fragments outside circle
    // }
    // fragColor = vec4(color, 1);
}
