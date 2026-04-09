#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 view;
uniform mat4 projection;
uniform vec2 screenSize; 

in VS_OUT {
    vec3 color;
    float opacity;
    vec3 scale;
    vec4 rot;
    vec4 viewPos;
} gs_in[];

out vec2 vTexCoord; 
out vec3 vColor;
out vec4 vConic;

mat3 quatToMat3(vec4 q) {
    float r = q.w; float x = q.x; float y = q.y; float z = q.z;
    return mat3(
        1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y + r * z), 2.0 * (x * z - r * y),
        2.0 * (x * y - r * z), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z + r * x),
        2.0 * (x * z + r * y), 2.0 * (y * z - r * x), 1.0 - 2.0 * (x * x + y * y)
    );
}

void main() {
    vec4 pView = gs_in[0].viewPos;
    if (pView.z > -0.1) return; 

    mat3 R = quatToMat3(gs_in[0].rot);
    mat3 S = mat3(0.0);
    S[0][0] = gs_in[0].scale.x;
    S[1][1] = gs_in[0].scale.y;
    S[2][2] = gs_in[0].scale.z;
    mat3 M = R * S;
    mat3 cov3D = M * transpose(M);

    mat3 viewRot = mat3(view[0].xyz, view[1].xyz, view[2].xyz);
    float focal = screenSize.y * projection[1][1] / 2.0; 
    
    float lim = -0.05; 
    float pz = min(lim, pView.z);
    mat3 J = mat3(
        focal / pz, 0.0, -(focal * pView.x) / (pz * pz),
        0.0, focal / pz, -(focal * pView.y) / (pz * pz),
        0.0, 0.0, 1.0 
    );
    
    mat3 T = J * viewRot;
    mat3 cov2D = T * cov3D * transpose(T);

    // 粒感軽減
    cov2D[0][0] += 0.9;
    cov2D[1][1] += 0.9;

    float det = cov2D[0][0] * cov2D[1][1] - cov2D[0][1] * cov2D[0][1];
    if (det <= 0.0001) return; 
    float det_inv = 1.0 / det;

    vConic = vec4(cov2D[1][1] * det_inv, -cov2D[0][1] * det_inv, cov2D[0][0] * det_inv, gs_in[0].opacity);
    vColor = gs_in[0].color;

    // --- Step3：固有値ベースの radius 計算（本家と同じ） ---
    float trace = cov2D[0][0] + cov2D[1][1];
    float disc  = sqrt(max(0.0, trace * trace - 4.0 * det));
    float lambda_max = 0.5 * (trace + disc);

    float radius = ceil(3.0 * sqrt(lambda_max));

    for (int i = 0; i < 4; i++) {
        vec2 offset = vec2((i % 2) * 2.0 - 1.0, (i / 2) * 2.0 - 1.0) * radius;
        vec4 pos = projection * pView;
        pos.xy += (offset / screenSize) * pos.w; 
        vTexCoord = offset;
        gl_Position = pos;
        EmitVertex();
    }
    EndPrimitive();
}