#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 view;
uniform mat4 projection;

in VS_OUT {
    vec3 scale;
    vec4 rot;
    vec3 color;
    float opacity;
} gs_in[];

out vec2 TexCoord;
out vec3 vColor;
out float vOpacity;

mat3 quatToMat3(vec4 q) {
    float r = q.w; float x = q.x; float y = q.y; float z = q.z;
    return mat3(
        1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y + r * z), 2.0 * (x * z - r * y),
        2.0 * (x * y - r * z), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z + r * x),
        2.0 * (x * z + r * y), 2.0 * (y * z - r * x), 1.0 - 2.0 * (x * x + y * y)
    );
}

void main() {
    // 粒子の大きさを調整 (modifierを大きくすると肉厚になります)
    float modifier = 2.0; 
    vec3 scale = exp(gs_in[0].scale) * modifier;
    mat3 R = quatToMat3(normalize(gs_in[0].rot));
    mat3 S = mat3(scale.x, 0, 0, 0, scale.y, 0, 0, 0, scale.z);
    mat3 M = R * S;

    vColor = gs_in[0].color;
    vOpacity = 1.0 / (1.0 + exp(-gs_in[0].opacity)); // Sigmoid

    for (int i = 0; i < 4; i++) {
        vec2 offset = vec2((i % 2) * 2.0 - 1.0, (i / 2) * 2.0 - 1.0);
        vec3 quadPos = M * vec3(offset, 0.0);
        
        // ビュー空間での計算（ビルボード化に近い処理）
        gl_Position = projection * view * (gl_in[0].gl_Position + vec4(quadPos, 0.0));
        TexCoord = offset;
        EmitVertex();
    }
    EndPrimitive();
}