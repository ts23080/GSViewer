#version 460 core

// C++側の Loading::GaussianSplat と完全に一致させる
struct GaussianSplat {
    vec3 pos;          // 0-2: x, y, z
    vec3 sh_base;      // 3-5: f_dc_0, 1, 2
    float sh_rest[48]; // 6-53: f_rest_0...47
    float opacity;     // 54
    vec3 scale;        // 55-57
    vec4 rot;          // 58-61 (Quaternion: x, y, z, w)
};

// Renderer.cpp で glBindBufferBase(..., 0, m_ssbo) とした「0」番を指定
layout(std430, binding = 0) buffer SplatBuffer {
    GaussianSplat splats[];
};

// ジオメトリシェーダーへ渡すデータ
out VS_OUT {
    vec3 pos;
    vec3 sh_base;
    float sh_rest[48];
    float opacity;
    vec3 scale;
    vec4 rot;
} vs_out;

void main() {
    // glDrawArrays(GL_POINTS, 0, numSplats) で渡された「何番目の点か」を取得
    uint idx = gl_VertexID;

    // SSBOからデータを読み出す
    GaussianSplat s = splats[idx];

    // データをそのままジオメトリシェーダーへ転送
    vs_out.pos      = s.pos;
    vs_out.sh_base  = s.sh_base;
    for(int i = 0; i < 48; i++) {
        vs_out.sh_rest[i] = s.sh_rest[i];
    }
    vs_out.opacity  = s.opacity;
    vs_out.scale    = s.scale;
    vs_out.rot      = s.rot;

    // 座標計算自体はジオメトリシェーダーで行うため、ここでは未変換の値を渡す
    gl_Position = vec4(s.pos, 1.0);
}