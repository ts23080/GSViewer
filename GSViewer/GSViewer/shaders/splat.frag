#version 460 core
precision highp float;

in vec2 vTexCoord;
in vec3 vColor;
in vec4 vConic; // GS側と型を一致させる
out vec4 FragColor;

// ACESトーンマッピング：実写に近い階調表現
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    // 公式ベースの正確なパワー計算
    float power = -0.5 * (vConic.x * vTexCoord.x * vTexCoord.x + 
                          vConic.z * vTexCoord.y * vTexCoord.y) 
                          - vConic.y * vTexCoord.x * vTexCoord.y;
    
    if (power > 0.0) discard;

    // vConic.w から不透明度を取得
    float alpha = vConic.w * exp(power);
    
    if (alpha < 0.04) discard; 
    alpha = min(0.99, alpha);

    // --- 色の調整：暗く、鮮やかに ---
    vec3 color = vColor;
    
    // 1. 彩度を少し強調
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luma), color, 1.5);

    // 2. 【核心】露出を大幅に下げて実写に近づける
    // 0.6 〜 0.7 程度にすると「白っぽさ」が抜けて重厚感が出ます
    color *= 0.5; 

    // 3. トーンマッピングとガンマ補正
    color = ACESFilm(color);
    vec3 sRGB = pow(color, vec3(1.0 / 2.2));

    // 4. 合成用出力 (Premultiplied Alpha)
    FragColor = vec4(sRGB, alpha);
}