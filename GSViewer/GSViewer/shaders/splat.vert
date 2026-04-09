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

    // --- 1. カメラ空間に変換 ---
    vec4 worldPos = vec4(s.px, s.py, s.pz, 1.0);
    vec4 camPos   = view * worldPos;

    // ジオメトリシェーダーで使うのは viewPos のみ
    vs_out.viewPos = camPos;

    // gl_Position は "点" として扱うので camPos を渡す
    gl_Position = camPos;

    // --- 2. SH0 の色計算 ---
    const float SH_C0 = 0.28209479177387814;
    vs_out.color = vec3(s.r, s.g, s.b) * SH_C0 + 0.5;

    // --- 3. 不透明度の補正 ---
    float k = 1.0;
    float b = 0.5;
    float rawOpacity = 1.0 / (1.0 + exp(-(k * s.opacity + b)));
    vs_out.opacity = sqrt(rawOpacity);

    // --- 4. スケールと回転（そのまま） ---
    vs_out.scale = exp(vec3(s.sx, s.sy, s.sz));
    vs_out.rot   = normalize(vec4(s.rx, s.ry, s.rz, s.rw));
}
