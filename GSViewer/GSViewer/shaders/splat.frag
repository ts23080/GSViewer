#version 460 core
in vec2 TexCoord;
in vec3 vColor;
in float vOpacity;
out vec4 FragColor;

void main() {
    float d = dot(TexCoord, TexCoord);
    if (d > 1.0) discard; // 円の外を捨てる
    float alpha = vOpacity * exp(-d * 4.0); // 中心ほど濃く、外側ほど薄く
    FragColor = vec4(vColor, alpha);
}