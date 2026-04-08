#version 460 core
in vec2 vTexCoord;
in vec3 vColor;
in float vOpacity;
in vec3 vConic;
out vec4 FragColor;

void main() {
    // 2Dガウス分布の計算: exp(-0.5 * (a*x^2 + 2b*xy + c*y^2))
    float power = -0.5 * (vConic.x * vTexCoord.x * vTexCoord.x + 
                          2.0 * vConic.y * vTexCoord.x * vTexCoord.y + 
                          vConic.z * vTexCoord.y * vTexCoord.y);
    
    if (power > 0.0) discard;
    float alpha = vOpacity * exp(power);
    
    if (alpha < 1.0/255.0) discard;

    // ガンマ補正をして出力
    vec3 unitColor = clamp(vColor, 0.0, 1.0);
    FragColor = vec4(unitColor * alpha, alpha); // Premultiplied Alpha
}