#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 view;
uniform mat4 projection;

in VS_OUT {
    vec3 color;
    float opacity;
    vec3 scale;
    vec4 rot;
} gs_in[];

out vec2 TexCoord;
out vec3 vColor;
out float vOpacity;

// クォータニオンを回転行列に変換
mat3 quatToMat3(vec4 q) {
    float r = q.w; float x = q.x; float y = q.y; float z = q.z;
    // GLSLのmat3は列優先(column-major)なので、C++側の行優先(row-major)と転置の関係にする
    return mat3(
        1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y - r * z), 2.0 * (x * z + r * y),
        2.0 * (x * y + r * z), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z - r * x),
        2.0 * (x * z - r * y), 2.0 * (y * z + r * x), 1.0 - 2.0 * (x * x + y * y)
    );
}

void main() {
    // 1. まずカメラ空間（View空間）での中心座標を計算
    // mat4 * vec4 の順番がOpenGLの標準
    vec4 viewCenter = view * vec4(gl_in[0].gl_Position.xyz, 1.0);

    // --- 【重要】カリング：カメラの背後にある点は処理しない ---
    // Eigenで作った行列の場合、Z方向が標準と逆（前方がZ正）の可能性があります。
    // トゲトゲが出る場合、この条件を inverse ( < 0.1 など) にしてみてください。
    // まずはより確実に、カメラの少し後ろ（Z=-0.2）より前にあるものだけ描画します。
    //if (viewCenter.z < -0.2) return; 

    // 2. スケールと回転を適用した変換行列 R
    vec3 scale = exp(gs_in[0].scale);
    mat3 R = quatToMat3(normalize(gs_in[0].rot));

    vColor = gs_in[0].color;
    // 不透明度の復元
    vOpacity = 1.0 / (1.0 + exp(-gs_in[0].opacity));

    // ガウスの広がり（サイズ）を定義
    float size = 3.0; 

    for (int i = 0; i < 4; i++) {
        // offset は -1.0 ～ 1.0
        vec2 offset = vec2((i % 2) * 2.0 - 1.0, (i / 2) * 2.0 - 1.0);
        
        // 3. 回転・スケールを適用したオフセットを計算
        // mat3 * vec3 の順番。scale.xy だけ使う
        vec3 localPos = R * vec3(offset.x * scale.x * size, offset.y * scale.y * size, 0.0);
        
        // 4. View空間で中心座標に足し合わせる
        vec4 pos = viewCenter + vec4(localPos, 0.0);
        
        // 5. 最後に投影
        gl_Position = projection * pos;
        
        TexCoord = offset;
        EmitVertex();
    }
    EndPrimitive();
}