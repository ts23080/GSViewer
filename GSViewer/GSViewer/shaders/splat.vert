#version 460 core

struct GaussianSplat {
    float px, py, pz;
    float r, g, b;
    float sh_rest[45];
    float opacity;
    float sx, sy, sz;
    float rx, ry, rz, rw;
};

layout(std430, binding = 0) readonly buffer SplatData { GaussianSplat splats[]; };
// Renderer側で binding = 1 にソート済みインデックスを渡す必要があります
layout(std430, binding = 1) readonly buffer SortData { uint sortedIndices[]; };

uniform mat4 view;
uniform mat4 projection;
uniform vec2 screenSize;
uniform float focalLength; // レンズの焦点距離（通常 w / (2 * tan(fov/2))）

out vec2 vTexCoord;
out vec3 vColor;
out vec4 vConic;

void main() {
    // 1. ソート済みインデックスに基づきデータを取得
    uint splatIdx = sortedIndices[gl_InstanceID];
    GaussianSplat s = splats[splatIdx];

    // 2. カメラ空間の位置計算
    vec4 camPos = view * vec4(s.px, s.py, s.pz, 1.0);
    float pz = camPos.z;
    
    // ニアクリップより後ろなら描画しない（画面外へ飛ばす）
    if (pz > -0.1) {
        gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }

// --- 3. 3D共分散行列の構築 ---
    float quat_x = s.rx; float quat_y = s.ry; float quat_z = s.rz; float quat_w = s.rw;
    
    // 回転行列の計算（変数を quat_ にして衝突を回避）
    mat3 R = mat3(
        1.0 - 2.0 * (quat_y * quat_y + quat_z * quat_z), 2.0 * (quat_x * quat_y - quat_w * quat_z), 2.0 * (quat_x * quat_z + quat_w * quat_y),
        2.0 * (quat_x * quat_y + quat_w * quat_z), 1.0 - 2.0 * (quat_x * quat_x + quat_z * quat_z), 2.0 * (quat_y * quat_z - quat_w * quat_x),
        2.0 * (quat_x * quat_z - quat_w * quat_y), 2.0 * (quat_y * quat_z + quat_w * quat_x), 1.0 - 2.0 * (quat_x * quat_x + quat_y * quat_y)
    );
    
    mat3 S = mat3(0.0);
    vec3 scale = exp(vec3(s.sx, s.sy, s.sz));
    S[0][0] = scale.x; S[1][1] = scale.y; S[2][2] = scale.z;
    
    mat3 M = R * S;
    mat3 cov3D = M * transpose(M);

    // --- 4. 2D投影 (EWA Approximation) ---
    float f = screenSize.y * 1.2; // 焦点距離
    mat3 J = mat3(
        f/pz, 0.0, -(f * camPos.x) / (pz * pz),
        0.0, f/pz, -(f * camPos.y) / (pz * pz),
        0.0, 0.0, 0.0
    );
    mat3 W = mat3(view);
    mat3 T = W * J;
    
    // エラーC7011の修正：mat3の結果から左上のmat2部分だけを明示的に取り出す
    mat3 cov2D_temp = transpose(T) * cov3D * T;
    mat2 cov2D = mat2(cov2D_temp[0].xy, cov2D_temp[1].xy);
    
    // Low-pass filter (トゲの鋭さを維持するため 0.1)
    cov2D[0][0] += 0.1;
    cov2D[1][1] += 0.1;

    // 5. 逆行列 (Conic) の計算
    float det = cov2D[0][0] * cov2D[1][1] - cov2D[0][1] * cov2D[0][1];
    vec3 conic = vec3(cov2D[1][1], -cov2D[0][1], cov2D[0][0]) / det;

    // 6. Quad頂点の生成 (Triangle Strip 用)
    // gl_VertexID 0:(-1,-1), 1:(1,-1), 2:(-1,1), 3:(1,1)
    vec2 offset = vec2(float(gl_VertexID % 2 == 1), float(gl_VertexID >= 2)) * 2.0 - 1.0;
    
    // 固有値から描画半径を決定
    float mid = 0.5 * (cov2D[0][0] + cov2D[1][1]);
    float lambda = mid + sqrt(max(0.1, mid * mid - det));
    float radius = ceil(3.0 * sqrt(lambda)); // 3σ

    vTexCoord = offset * radius;
    vColor = max(vec3(0.0), 0.5 + (0.282094 * vec3(s.r, s.g, s.b)));
    vConic = vec4(conic, 1.0 / (1.0 + exp(-(s.opacity + 3.0)))); // opacityブースト込

    // スクリーン空間へ投影
    vec4 clipPos = projection * camPos;
    clipPos.xy += offset * radius * (2.0 / screenSize) * clipPos.w;
    gl_Position = clipPos;
}