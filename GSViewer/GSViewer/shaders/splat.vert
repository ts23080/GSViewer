#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aOpacity;
layout (location = 2) in vec3 aScale;
layout (location = 3) in vec4 aRot;
layout (location = 4) in vec3 aColor; // PLYから読み取った色

out VS_OUT {
    vec3 scale;
    vec4 rot;
    vec3 color;
    float opacity;
} vs_out;

void main() {
    gl_Position = vec4(aPos, 1.0);
    vs_out.scale = aScale;
    vs_out.rot = aRot;
    vs_out.color = aColor;
    vs_out.opacity = aOpacity;
}