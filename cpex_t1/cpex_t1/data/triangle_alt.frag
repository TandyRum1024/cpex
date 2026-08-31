#version 450 core
out vec4 FragColor;

in vec2 vUv;

void main() {
    vec3 tint = vec3(vUv.xy * 0.5 + 0.5, 0.5);

    FragColor = vec4(tint.rgb, 1.0);
}