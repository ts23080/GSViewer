#version 460 core
precision highp float;

in vec2 vTexCoord;
in vec3 vColor;
in vec4 vConic; 
out vec4 FragColor;

void main() {
    // 1. ガウス関数の指数計算 (ここは正しく修正されています)
    float power = -0.5 * (vConic.x * vTexCoord.x * vTexCoord.x + 
                          vConic.z * vTexCoord.y * vTexCoord.y) 
                          - vConic.y * vTexCoord.x * vTexCoord.y;
    
    if (power > 0.0) discard;

    // 2. アルファ値の計算
    float alpha = vConic.w * exp(power * 2.0);
    
    // 論文実装では 1/255 (約0.0039) 以下の微小なアルファは破棄して高速化します
    if (alpha < 0.1) discard; 
    alpha = min(0.99, alpha);

    // 3. 色の出力
    // 【重要】ここではトーンマッピングやガンマ補正を「行わない」
    // vColor は .vert で 0.5 + 0.28*coeff と計算されているため、すでに線形空間にあります
    vec3 color = vColor;

    // 4. Premultiplied Alpha（事前乗算アルファ）
    // 3DGSの公式実装および論文のブレンド式に合わせるため、
    // 出力段階で alpha を掛けておきます。
    FragColor = vec4(color * alpha, alpha);
}