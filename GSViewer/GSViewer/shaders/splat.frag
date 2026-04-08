#version 460 core
in vec2 TexCoord;
in vec3 vColor;
in float vOpacity;
out vec4 FragColor;

void main() {
    // 中心からの距離の2乗を計算
    float d = dot(TexCoord, TexCoord);
    
    // 円の外（距離 > 1.0）を捨てる
    if (d > 1.0) discard; 

    // ガウス関数（中心ほど濃く、外側ほど薄く）
    // -d * 4.0 は、ぼかし具合を調整する定数。4.0～8.0程度が良い
    float power = exp(-0.5 * d * 6.0); 
    float alpha = vOpacity * power;

    // アルファブレンディングが効くように、FragColorを書き出す
    FragColor = vec4(vColor, alpha);
}