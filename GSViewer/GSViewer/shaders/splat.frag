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
    
    if (alpha < 0.005) discard; 

    vec3 color = vColor;

    FragColor = vec4(color * alpha, alpha);
}