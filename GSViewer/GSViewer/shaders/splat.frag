#version 460 core
precision highp float;

in vec2 vTexCoord;
in vec3 vColor;
in vec4 vConic; 
out vec4 FragColor;

void main() {
    float power = -0.5 * (vConic.x * vTexCoord.x * vTexCoord.x + 
                          vConic.z * vTexCoord.y * vTexCoord.y) 
                          - vConic.y * vTexCoord.x * vTexCoord.y;
    
    if (power > 0.0) discard;

    float alpha = vConic.w * exp(power);
    
    if (alpha < 0.005) discard; 

    // --- 彩度アップ処理 ---
    vec3 color = vColor;
    // 1. 露出（明るさ）を上げる 
    // 1.2 〜 1.5 程度で調整。上げすぎると白飛びするので注意。
    color *= 1.5;
    
    // 1. 輝度（明るさの基準）を計算
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    // 2. 彩度を強める (1.5を上げるとより鮮やかに、1.0が元通り)
    color = mix(vec3(luma), color, 2.0); 
    
    // 3. マイナス値や白飛びを抑えるクランプ
    color = clamp(color, 0.0, 1.0);
    // ----------------------

    FragColor = vec4(color * alpha, alpha);
}