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

mat3 quatToMat3(vec4 q) {
    float r = q.w; float x = q.x; float y = q.y; float z = q.z;
    return mat3(
        1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y + r * z), 2.0 * (x * z - r * y),
        2.0 * (x * y - r * z), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z + r * x),
        2.0 * (x * z + r * y), 2.0 * (y * z - r * x), 1.0 - 2.0 * (x * x + y * y)
    );
}

void main() {
    // 1. まずカメラ空間（View空間）での中心座標を計算
    vec4 viewCenter = view * gl_in[0].gl_Position;

    // --- 【超重要】カリング：カメラの背後にある点は処理しない ---
    // これがないと、カメラの後ろにある点が画面中央に向かって線を引きまくります
    if (viewCenter.z > -0.1) return; 

    // 2. スケールと回転を適用した変換行列 M
    vec3 scale = exp(gs_in[0].scale);
    mat3 R = quatToMat3(normalize(gs_in[0].rot));
    
    // 3. ビルボードのサイズ調整（定数倍して見やすくします）
    // 3DGSでは Sigma (共分散) を計算しますが、まずは簡易的に 2.0 倍程度の広がりを持たせます
    float size = 2.0; 

    vColor = gs_in[0].color;
    vOpacity = 1.0 / (1.0 + exp(-gs_in[0].opacity));

    for (int i = 0; i < 4; i++) {
        // offset は -1.0 ～ 1.0
        vec2 offset = vec2((i % 2) * 2.0 - 1.0, (i / 2) * 2.0 - 1.0);
        
        // 4. 回転・スケールを適用したオフセットを計算
        // ここで R * (scale * offset) の形にするのが正解です
        vec3 localPos = R * (scale * vec3(offset * size, 0.0));
        
        // 5. View空間で中心座標に足し合わせる
        vec4 pos = viewCenter + vec4(localPos, 0.0);
        
        // 6. 最後に投影
        gl_Position = projection * pos;
        
        TexCoord = offset;
        EmitVertex();
    }
    EndPrimitive();
}