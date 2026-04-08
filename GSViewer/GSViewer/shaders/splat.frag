#version 460 core
in vec2 TexCoord;
in vec3 vColor;
in float vOpacity;
out vec4 FragColor;

void main() {
    // 中心からの距離の2乗
    float r2 = dot(TexCoord, TexCoord);
    if (r2 > 1.0) discard; // 円の外側を捨てる

    // ガウス関数による減衰
    float power = exp(-0.5 * r2 * 10.0); // 10.0は広がり調整
    float alpha = vOpacity * power;

    if (alpha < 0.1) discard; // 薄すぎる部分は描画しない

    // SHの色(f_dc)を簡易的にRGBとして表示
    // ※本来は 0.5 + 0.28209 * f_dc などの補正が必要
    vec3 color = vColor * 0.28209 + 0.5;
    
    FragColor = vec4(color, alpha);
}