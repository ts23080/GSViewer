#version 460 core

in GS_OUT {
    vec4 color;
    vec2 texCoord;   // 四角形内の座標 (-3.0 to 3.0)
    flat mat2 conic; // 楕円の形状（逆共分散行列）
} fs_in;

out vec4 fragColor;

void main() {
    // 1. 楕円の指数部分 (d^2) を計算
    // 二次形式: d^2 = [x, y] * Conic * [x, y]^T
    vec2 d = fs_in.texCoord;
    float power = -0.5 * (fs_in.conic[0][0] * d.x * d.x + 
                          fs_in.conic[1][1] * d.y * d.y + 
                          (fs_in.conic[0][1] + fs_in.conic[1][0]) * d.x * d.y);

    // 2. ガウス分布の計算
    // 指数が大きすぎると（中心から遠すぎると）透明にする
    if (power > 0.0) discard;
    
    float alpha = exp(power) * fs_in.color.a;

    // 3. アルファ値が低すぎる場合は描画をスキップ（最適化）
    if (alpha < 1.0/255.0) discard;

    // 4. 最終的な色を出力
    // RGBには既に不透明度が乗算されている前提（Pre-multiplied Alpha）
    fragColor = vec4(fs_in.color.rgb * alpha, alpha);
}