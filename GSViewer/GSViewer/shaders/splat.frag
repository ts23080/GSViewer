#version 460 core
in vec2 TexCoord;
in vec3 vColor;
in float vOpacity;
out vec4 FragColor;

void main() {
    float r2 = dot(TexCoord, TexCoord);
    if (r2 > 1.0) discard;

    // ガウス減衰。2.0を小さくするとより「ふんわり」します
    float power = exp(-2.0 * r2);
    float alpha = power * vOpacity;

    if (alpha < 0.01) discard;
    FragColor = vec4(vColor, alpha);
}