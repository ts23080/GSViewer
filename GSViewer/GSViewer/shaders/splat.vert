#version 460 core

struct GaussianSplat {
    vec3 pos;
    vec3 color;
    float sh_rest[45];
    float opacity;
    vec3 scale;
    vec4 rot;
};

layout(std430, binding = 0) readonly buffer SplatData {
    GaussianSplat splats[];
};

uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
    vec3 color;
    float opacity;
    vec3 scale;
    vec4 rot;
} vs_out;

void main() {
    // gl_VertexID は EBO を通した後のインデックス
    GaussianSplat s = splats[gl_VertexID];

    gl_Position = vec4(s.pos, 1.0);
    vs_out.color = s.color;
    vs_out.opacity = s.opacity;
    vs_out.scale = s.scale;
    vs_out.rot = s.rot;
}