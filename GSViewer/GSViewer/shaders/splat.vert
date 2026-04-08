#version 460 core

// 1. メモリのズレ（パディング）を防ぐため、vec3を使わず全てfloatで定義する
struct GaussianSplat {
    float px, py, pz;
    float r, g, b;
    float sh_rest[45];
    float opacity;
    float sx, sy, sz;
    float rx, ry, rz, rw;
};

// バインドポイントなどは Renderer::Setup と合わせてください
layout(std430, binding = 0) readonly buffer SplatData {
    GaussianSplat splats[];
};

out VS_OUT {
    vec3 color;
    float opacity;
    vec3 scale;
    vec4 rot;
} vs_out;

void main() {
    // EBO（インデックスバッファ）を使っている場合、gl_VertexID はソート済みインデックスになる
    GaussianSplat s = splats[gl_VertexID];

    // 2. float を vec3/vec4 に組み立て直して渡す
    gl_Position = vec4(s.px, s.py, s.pz, 1.0);
    
    vs_out.color   = vec3(s.r, s.g, s.b);
    vs_out.opacity = s.opacity;
    vs_out.scale   = vec3(s.sx, s.sy, s.sz);
    vs_out.rot     = vec4(s.rx, s.ry, s.rz, s.rw);
}