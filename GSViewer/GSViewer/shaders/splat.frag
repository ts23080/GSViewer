#version 460 core
in vec2 vTexCoord;
in vec3 vColor;
in float vOpacity;
in vec3 vConic;
out vec4 FragColor;

// ACESトーンマッピング近似関数（写真のような階調を作る）
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    float power = -0.5 * (vConic.x * vTexCoord.x * vTexCoord.x + 
                          2.0 * vConic.y * vTexCoord.x * vTexCoord.y + 
                          vConic.z * vTexCoord.y * vTexCoord.y);
    
    if (power > 0.0) discard;

    // ガウスの広がり（0.8〜0.9が密度とクッキリ感のバランスが良い）
    float G = exp(power * 0.9);
    float alpha = clamp(vOpacity * G, 0.0, 0.99);
    if (alpha < 0.04) discard; 

    // --- 【色彩設計】 ---
    vec3 color = vColor;

    // 1. 彩度を「さらに」強化 (1.5倍)
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luma), color, 1.5);

    // 2. ACESトーンマッピング適用
    // これにより、色が深く、実写映画のような質感になります
    color = ACESFilm(color * 0.5); // 0.5は露出補正（明るさの微調整）

    // 3. ガンマ補正 (sRGB)
    vec3 sRGB = pow(color, vec3(1.0 / 2.2));

    // 4. コントラスト補正
    // わずかにコントラストを上げ、中間色をパキッとさせます
    sRGB = mix(sRGB, sRGB * sRGB * (3.0 - 2.0 * sRGB), 0.5);

    // Premultiplied Alpha 出力
    FragColor = vec4(sRGB * alpha, alpha);
}