#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 view;
uniform mat4 projection;
uniform vec2 screenSize; // Rendererから渡す (width, height)

in VS_OUT {
    vec3 color;
    float opacity;
    vec3 scale;
    vec4 rot;
    vec4 viewPos;
} gs_in[];

out vec2 vTexCoord; // 楕円内での座標 (-3.0 ～ 3.0)
out vec3 vColor;
out float vOpacity;
out vec3 vConic;    // 楕円の形状パラメータ (a, b, c)

// クォータニオンを回転行列に変換
mat3 quatToMat3(vec4 q) {
    float r = q.w; float x = q.x; float y = q.y; float z = q.z;
    return mat3(
        1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y - r * z), 2.0 * (x * z + r * y),
        2.0 * (x * y + r * z), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z - r * x),
        2.0 * (x * z - r * y), 2.0 * (y * z + r * x), 1.0 - 2.0 * (x * x + y * y)
    );
}

void main() {
    vec4 pView = gs_in[0].viewPos;
    if (pView.z > -0.1) return; // ニアクリップ

    // 1. 3D共分散行列の計算 (Σ = R * S * S^T * R^T)
    mat3 R = quatToMat3(gs_in[0].rot);
    mat3 S = mat3(0.0);
    S[0][0] = gs_in[0].scale.x;
    S[1][1] = gs_in[0].scale.y;
    S[2][2] = gs_in[0].scale.z;
    mat3 M = R * S;
    mat3 cov3D = M * transpose(M);

    // 2. 2Dへの投影 (EWA Splattingの近似)
    float focal = screenSize.y * projection[1][1] / 2.0; // 焦点距離の推定
    mat3 J = mat3(
        focal / pView.z, 0.0, -(focal * pView.x) / (pView.z * pView.z),
        0.0, focal / pView.z, -(focal * pView.y) / (pView.z * pView.z),
        0.0, 0.0, 0.0
    );
    mat3 W = transpose(mat3(view)); // Rendererから渡されるviewの回転成分
    mat3 T = J * transpose(mat3(view));
    mat3 cov2D = T * cov3D * transpose(T);

    // 3. 2D共分散の逆行列 (Conic) を計算してフラグメントへ
    // 安定化のための微小値を加算
    cov2D[0][0] += 0.3;
    cov2D[1][1] += 0.3;
    float det = cov2D[0][0] * cov2D[1][1] - cov2D[0][1] * cov2D[1][0];
    if (det == 0.0) return;
    float det_inv = 1.0 / det;
    vConic = vec3(cov2D[1][1] * det_inv, -cov2D[0][1] * det_inv, cov2D[0][0] * det_inv);

    vColor = gs_in[0].color;
    vOpacity = gs_in[0].opacity;

    // 楕円の大きさに合わせた四角形の生成 (3シグマ範囲)
    float mid = 0.5 * (cov2D[0][0] + cov2D[1][1]);
    float term = sqrt(max(0.1, mid * mid - det));
    float lambda1 = mid + term;
    float lambda2 = mid - term;
    float radius = ceil(3.0 * sqrt(max(lambda1, lambda2))); // 影響範囲

    for (int i = 0; i < 4; i++) {
        vec2 offset = vec2((i % 2) * 2.0 - 1.0, (i / 2) * 2.0 - 1.0) * radius;
        vec4 pos = projection * pView;
        pos.xy += offset / screenSize * pos.w * 2.0; 
        vTexCoord = offset;
        gl_Position = pos;
        EmitVertex();
    }
    EndPrimitive();
}