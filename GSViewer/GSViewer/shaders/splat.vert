#version 460 core

struct GaussianSplat {
    float px, py, pz;
    float r, g, b;
    float sh_rest[45]; // 構造体の定義はバッファのオフセット維持のため残す
    float opacity;
    float sx, sy, sz;
    float rx, ry, rz, rw;
};

layout(std430, binding = 0) readonly buffer SplatData { GaussianSplat splats[]; };
layout(std430, binding = 1) readonly buffer SortData { uint sortedIndices[]; };

uniform mat4 view;
uniform mat4 projection;
uniform vec2 screenSize;

out vec2 vTexCoord;
out vec3 vColor;
out vec4 vConic;

void main() {
    uint splatIdx = sortedIndices[gl_InstanceID];
    GaussianSplat s = splats[splatIdx];

    // 1. 位置計算
    vec4 worldPos = vec4(s.px, s.py, s.pz, 1.0);
    vec4 camPos = view * worldPos;
    float pz = camPos.z;
    
    if (pz > -0.1) {
        gl_Position = vec4(0.0);
        return;
    }

    // 2. 3D共分散 (回転とスケール)
    vec4 q = normalize(vec4(s.rw, s.rx, s.ry, s.rz));
    mat3 R = mat3(
        1.0 - 2.0 * (q.y * q.y + q.z * q.z), 2.0 * (q.x * q.y - q.w * q.z), 2.0 * (q.x * q.z + q.w * q.y),
        2.0 * (q.x * q.y + q.w * q.z), 1.0 - 2.0 * (q.x * q.x + q.z * q.z), 2.0 * (q.y * q.z - q.w * q.x),
        2.0 * (q.x * q.z - q.w * q.y), 2.0 * (q.y * q.z + q.w * q.x), 1.0 - 2.0 * (q.x * q.x + q.y * q.y)
    );
    mat3 S = mat3(0.0);
    vec3 scale = exp(vec3(s.sx, s.sy, s.sz));
    S[0][0] = scale.x; S[1][1] = scale.y; S[2][2] = scale.z;
    mat3 M = R * S;
    mat3 cov3D = M * transpose(M);

    // 3. 2D投影 (EWA)
    float f = screenSize.y * 1.2;
    mat3 J = mat3(f/pz, 0.0, -(f*camPos.x)/(pz*pz), 0.0, f/pz, -(f*camPos.y)/(pz*pz), 0.0, 0.0, 0.0);
    mat3 W = mat3(view);
    mat3 T = J * W;
    mat3 cov2D_temp = T * cov3D * transpose(T);
    mat2 cov2D = mat2(cov2D_temp[0].xy, cov2D_temp[1].xy) + mat2(0.1, 0.0, 0.0, 0.1);

    // 4. 逆行列・半径
    float det = cov2D[0][0] * cov2D[1][1] - cov2D[0][1] * cov2D[0][1];
    vec3 conic = vec3(cov2D[1][1], -cov2D[0][1], cov2D[0][0]) / det;
    float mid = 0.5 * (cov2D[0][0] + cov2D[1][1]);
    float lambda = mid + sqrt(max(0.1, mid * mid - det));
    float radius = ceil(3.0 * sqrt(lambda));

    // --- 5. 色の決定 (SHを使わないシンプルな形) ---
    // SH_C0 定数を用いて基本色を復元（通常、学習済みデータのDC成分はSH_C0で除算されているため）
    const float SH_C0 = 0.28209479;
    vColor = max(vec3(0.0), 0.5 + (SH_C0 * vec3(s.r, s.g, s.b)));
    
    vTexCoord = (vec2(float(gl_VertexID % 2 == 1), float(gl_VertexID >= 2)) * 2.0 - 1.0) * radius;
    vConic = vec4(conic, 1.0 / (1.0 + exp(-(s.opacity))));

    // 6. 投影
    vec4 clipPos = projection * camPos;
    clipPos.xy += (vTexCoord / radius) * radius * (2.0 / screenSize) * clipPos.w;
    gl_Position = clipPos;
}