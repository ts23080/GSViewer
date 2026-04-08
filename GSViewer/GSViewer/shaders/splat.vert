#version 460 core

struct GaussianSplat {
    float px, py, pz;
    float r, g, b;
    float sh_rest[45];
    float opacity;
    float sx, sy, sz;
    float rx, ry, rz, rw;
};

layout(std430, binding = 0) readonly buffer SplatData {
    GaussianSplat splats[];
};

uniform mat4 view;

out VS_OUT {
    vec3 color;
    float opacity;
    vec3 scale;
    vec4 rot;
    vec4 viewPos; // カメラ空間での位置
} vs_out;

void main() {
    GaussianSplat s = splats[gl_VertexID];
    vec4 worldPos = vec4(s.px, s.py, s.pz, 1.0);
    
    vs_out.viewPos = view * worldPos;
    gl_Position = worldPos; // ジオメトリシェーダーに渡す

    // SH 0次の計算（公式の係数 0.28209...）
    const float SH_C0 = 0.28209479177387814;
    vs_out.color = vec3(s.r, s.g, s.b) * SH_C0 + 0.5;
    
    vs_out.opacity = 1.0 / (1.0 + exp(-s.opacity)); // シグモイド
    vs_out.scale   = exp(vec3(s.sx, s.sy, s.sz));    // 指数関数
    vs_out.rot     = normalize(vec4(s.rx, s.ry, s.rz, s.rw));
}