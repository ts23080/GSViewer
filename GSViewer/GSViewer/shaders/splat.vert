#version 460 core

struct GaussianSplat {
    float px, py, pz;
    float r, g, b;
    float sh_rest[45];
    float opacity;
    float sx, sy, sz;
    float rx, ry, rz, rw; // C++側で x, y, z, w の順に格納されている想定
};

layout(std430, binding = 0) readonly buffer SplatData { GaussianSplat splats[]; };
layout(std430, binding = 1) readonly buffer SortData { uint sortedIndices[]; };

uniform mat4 view;
uniform mat4 projection;
uniform vec2 screenSize;

out vec2 vTexCoord;
out vec3 vColor;
out vec4 vConic; // xyz: conic, w: opacity

void main() {
    uint splatIdx = sortedIndices[gl_InstanceID];
    GaussianSplat s = splats[splatIdx];

    // 1. 位置計算
    vec4 worldPos = vec4(s.px, s.py, s.pz, 1.0);
    vec4 camPos = view * worldPos;
    float pz = camPos.z;
    
    // ニアクリップ（カメラのすぐ後ろの粒子を捨てる）
    if (pz > -0.1) {
        gl_Position = vec4(0.0);
        return;
    }

    // 2. 3D共分散 (回転とスケール)
    // C++側での代入: s.rw=w, s.rx=x, s.ry=y, s.rz=z
    float qx = s.rx; 
    float qy = s.ry; 
    float qz = s.rz; 
    float qw = s.rw;

    // 回転行列 R (標準的な公式)
    mat3 R = mat3(
        1.0 - 2.0 * (qy * qy + qz * qz), 2.0 * (qx * qy + qw * qz), 2.0 * (qx * qz - qw * qy),
        2.0 * (qx * qy - qw * qz), 1.0 - 2.0 * (qx * qx + qz * qz), 2.0 * (qy * qz + qw * qx),
        2.0 * (qx * qz + qw * qy), 2.0 * (qy * qz - qw * qx), 1.0 - 2.0 * (qx * qx + qy * qy)
    );

    // スケール計算 (expをとって正の値にする)
    vec3 scale = exp(vec3(s.sx, s.sy, s.sz));
    mat3 S = mat3(
        scale.x, 0.0, 0.0,
        0.0, scale.y, 0.0,
        0.0, 0.0, scale.z
    );
    
    // 3D共分散行列 Σ = R S S^T R^T
    mat3 M = R * S;
    mat3 cov3D = M * transpose(M);

// --- 3. 2D投影 (EWA) の修正 ---
    
    // 焦点距離の計算（projection[1][1]は 1.0 / tan(fov/2) に相当）
    float focal = screenSize.y * projection[1][1] / 2.0;
    
    // ヤコビ行列 J (列優先での定義)
    // カメラ空間の座標 (x, y, z) をスクリーン空間の (u, v) に投影する際の微分
    mat3 J = mat3(
        focal / pz, 0.0, 0.0,
        0.0, focal / pz, 0.0,
        -(focal * camPos.x) / (pz * pz), -(focal * camPos.y) / (pz * pz), 0.0
    );
    
    // ビュー行列の回転・平行移動成分
    mat3 W = mat3(view);
    
    // T = J * W を計算
    // 数学的には Σ' = (J*W) * Σ * (J*W)^T となる
    mat3 T = J * W;
    
    // 2D共分散行列の計算
    // cov3D は既に R*S*S^T*R^T で計算されている想定
    mat3 cov2D_full = T * cov3D * transpose(T);

    // 低パスフィルタ（アンチエイリアス）
    // 視点移動時のチラつきを抑えるため、対角成分に微小な値を加算
    mat2 cov2D = mat2(cov2D_full[0].xy, cov2D_full[1].xy) + mat2(0.3, 0.0, 0.0, 0.3);

    // 4. 逆行列（Conic）と半径
    float det = cov2D[0][0] * cov2D[1][1] - cov2D[0][1] * cov2D[0][1];
    if (det <= 0.0) {
        gl_Position = vec4(0.0);
        return;
    }
    
    // 楕円を描画するための係数
    vec3 conic = vec3(cov2D[1][1], -cov2D[0][1], cov2D[0][0]) / det;
    
    // 固有値から描画半径（ピクセル単位）を決定
    float mid = 0.5 * (cov2D[0][0] + cov2D[1][1]);
    float term = sqrt(max(0.1, mid * mid - det));
    float lambda = mid + term;
    float radius = ceil(3.0 * sqrt(lambda));

    // 5. 色と不透明度
    const float SH_C0 = 0.28209479;
    vColor = max(vec3(0.0), 0.5 + (SH_C0 * vec3(s.r, s.g, s.b)));
    
    // 不透明度の適用
    float op = 1.0 / (1.0 + exp(-s.opacity));
    vConic = vec4(conic, op); 

    // 6. 頂点生成
    vec2 quad[4] = vec2[](vec2(-1, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1));
    vec2 offset = quad[gl_VertexID % 4] * radius;
    
    vTexCoord = offset;
    vec4 clipPos = projection * camPos;
    // スクリーン座標へのオフセット適用
    clipPos.xy += (offset / screenSize) * 2.0 * clipPos.w;
    gl_Position = clipPos;
}