#version 460 core

// 構造体の並びに合わせたレイアウト設定
layout (location = 0) in vec3 aPos;     // px, py, pz
layout (location = 1) in vec3 aColor;   // r, g, b (SH基礎)
// layout (location = 2) は sh_rest[45] なので、シェーダーでは飛ばすことが多い
layout (location = 3) in float aOpacity; // opacity
layout (location = 4) in vec3 aScale;   // sx, sy, sz
layout (location = 5) in vec4 aRot;     // rx, ry, rz, rw

out VS_OUT {
    vec3 color;
    float opacity;
    vec3 scale;
    vec4 rot;
} vs_out;

uniform mat4 view;
uniform mat4 projection;

void main() {
    // 座標変換はジオメトリシェーダーで行うため、ここでは生データを渡す
    gl_Position = vec4(aPos, 1.0);
    vs_out.color = aColor;
    vs_out.opacity = aOpacity;
    vs_out.scale = aScale;
    vs_out.rot = aRot;
}