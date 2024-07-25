#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in float aOpacity;
layout(location = 3) in vec3 aCovA;
layout(location = 4) in vec3 aCovB;

uniform float W;
uniform float H;
uniform float focal_x;
uniform float focal_y;
uniform float tan_fovx;
uniform float tan_fovy;
uniform float scaleFactor;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 camMatrix;

out vec3 GeoColor;
out vec2 GeoPointImage;
out vec4 Geocon_o;
out float GeoRadius;
out float GeoScaleModif;

vec3 computeCov2D(vec3 mean, float[6] cov3DV, mat4 vm)
{
    vec4 t = vm * vec4(mean, 1.0);
    mat3 J = mat3(
        focal_x/t.z, 0, -focal_x * t.x / (t.z * t.z),
        0, focal_y/t.z, -focal_y * t.y / (t.z * t.z),
        0, 0, 0
    );
    mat3 W = mat3(
        vm[0][0], vm[1][0], vm[2][0],
        vm[0][1], vm[1][1], vm[2][1],
        vm[0][2], vm[1][2], vm[2][2]
    );

    mat3 T = W * J;
    mat3 Vrk = mat3(
        cov3DV[0], cov3DV[1], cov3DV[2],
        cov3DV[1], cov3DV[3], cov3DV[4],
        cov3DV[2], cov3DV[4], cov3DV[5]
    );

    mat3 cov = transpose(T) * transpose(Vrk) * T;
    // mat3 cov = W * Vrk * transpose(W) ;
    cov[0][0] += .3;
    cov[1][1] += .3;
    return vec3(cov[0][0], cov[0][1], cov[1][1]);

}

float ndc2Pix(float v, float S) {
    return ((v + 1.) * S - 1.) * .5;
}

void main()
{
    vec4 p4 = camMatrix * modelMatrix * vec4(aPos, 1.0);
    float pw = 1.0 / (p4.w + 1e-7);
    vec3 p_proj = p4.xyz * pw;

    // near culling
    if (p4.z < 0.1)
    {
        gl_Position = vec4(0, 0, 0, 1);
        return;
    }

    float cov3D[6] = float[6](aCovA.x, aCovA.y, aCovA.z, aCovB.x, aCovB.y, aCovB.z);

    mat4 vm = viewMatrix * modelMatrix;
    vec3 cov2D = computeCov2D(aPos, cov3D, vm);

    float det = cov2D.x * cov2D.z - cov2D.y * cov2D.y;
    if (det == 0.)
    {
        gl_Position = vec4(0, 0, 0, 1);
        return;
    }
    float det_inv = 1.0 / det;
    vec3 conic = vec3(cov2D.z, -cov2D.y, cov2D.x) * det_inv;

    float mid = 0.5 * (cov2D.x + cov2D.z);
    float lambda1  = mid + sqrt(max(0.1, mid * mid - det));
    float lambda2  = mid - sqrt(max(0.1, mid * mid - det));
    float radius = ceil(3. * sqrt(max(lambda1, lambda2)));
    vec2 point_image = vec2(ndc2Pix(p_proj.x, W), ndc2Pix(p_proj.y, H)); // like pixel on screen

    radius *= .15 + scaleFactor * .85;

    GeoColor = aColor;
    GeoRadius = radius;
    GeoPointImage = point_image;
    Geocon_o = vec4(conic, aOpacity);
    GeoScaleModif = 1. / scaleFactor;
}
