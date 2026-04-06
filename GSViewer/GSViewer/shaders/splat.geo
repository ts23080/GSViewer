#version 460 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// バーテックスシェーダーから受け取るデータ
in VS_OUT {
    vec3 pos;
    vec3 sh_base;
    float sh_rest[48];
    float opacity;
    vec3 scale;
    vec4 rot;
} gs_in[];

// フラグメントシェーダーへ渡すデータ
out GS_OUT {
    vec4 color;
    vec2 texCoord; // 四角形内での座標 (-2.0 to 2.0)
    flat mat2 conic; // 楕円の形状係数（逆共分散行列）
} gs_out;

// ユニフォーム変数（C++側から渡す行列）
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;
uniform vec2 screenSize;

// クォータニオンから回転行列(3x3)を作る関数
mat3 quatToRot(vec4 q) {
    float x = q.x; float y = q.y; float z = q.z; float w = q.w;
    return mat3(
        1.0 - 2.0*(y*y + z*z), 2.0*(x*y - w*z), 2.0*(x*z + w*y),
        2.0*(x*y + w*z), 1.0 - 2.0*(x*x + z*z), 2.0*(y*z - w*x),
        2.0*(x*z - w*y), 2.0*(y*z + w*x), 1.0 - 2.0*(x*x + y*y)
    );
}

void main() {
    // 1. 3D共分散行列 Sigma の計算 (S * R^T * R * S)
    mat3 R = quatToRot(gs_in[0].rot);
    mat3 S = mat3(0.0);
    S[0][0] = exp(gs_in[0].scale.x); // スケールは通常 log 空間なので exp
    S[1][1] = exp(gs_in[0].scale.y);
    S[2][2] = exp(gs_in[0].scale.z);
    
    mat3 M = R * S;
    mat3 Sigma = M * transpose(M);

    // 2. 視点座標系への変換
    vec4 posView = view * vec4(gs_in[0].pos, 1.0);
    
    // カメラの裏側なら描画しない
    if (posView.z > -0.1) return;

    // 3. 2Dへの投影（EWA Splatting の近似）
    // 本来はヤコビアン行列を用いた複雑な投影が必要ですが、
    // ここでは簡易的に画面上の広がりを計算します
    float focal = screenSize.y * projection[1][1] / 2.0;
    mat3 J = mat3(
        focal / posView.z, 0.0, -(focal * posView.x) / (posView.z * posView.z),
        0.0, focal / posView.z, -(focal * posView.y) / (posView.z * posView.z),
        0.0, 0.0, 0.0
    );
    mat3 W = mat3(view);
    mat3 T = J * W;
    mat2 cov2d = mat2(T * Sigma * transpose(T));
    
    // 4. 楕円の広がり（逆行列）を計算
    float det = cov2d[0][0] * cov2d[1][1] - cov2d[0][1] * cov2d[1][0];
    if (det == 0.0) return;
    gs_out.conic = mat2(cov2d[1][1], -cov2d[0][1], -cov2d[1][0], cov2d[0][0]) * (1.0 / det);

    // 5. 色の計算（簡易版：SHの0次のみ使用）
    // 本来は sh_rest を使い視線方向との演算が必要ですが、まずは基礎色で表示
    float SH_C0 = 0.28209479177387814f;
    gs_out.color = vec4(0.5 + SH_C0 * gs_in[0].sh_base, 1.0 / (1.0 + exp(-gs_in[0].opacity)));

    // 6. 四角形（ビルボード）の生成
    float radius = 3.0 * sqrt(max(cov2d[0][0], cov2d[1][1])); // 3シグマ分広げる
    vec2 extent = vec2(radius) / screenSize * 2.0;

    for (int i = 0; i < 4; i++) {
        vec2 offset = vec2((i & 1) == 0 ? -1.0 : 1.0, (i & 2) == 0 ? -1.0 : 1.0);
        gs_out.texCoord = offset * 3.0; // 余裕を持って広めに設定
        vec4 projPos = projection * posView;
        projPos.xy += offset * extent * projPos.w;
        gl_Position = projPos;
        EmitVertex();
    }
    EndPrimitive();
}