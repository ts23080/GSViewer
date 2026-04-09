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
    
    // --- 【別アプローチ】トゲを痩せさせる ---
    alpha = pow(alpha, 1.5); // 1.0より大きくすると鋭利になります
    
    if (alpha < 0.1) discard;
    FragColor = vec4(vColor * alpha, alpha);
}