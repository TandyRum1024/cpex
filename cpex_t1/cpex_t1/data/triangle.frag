#version 450 core
out vec4 FragColor;

in vec2 vUv;

uniform vec4 uTint;

void main() {
    vec3 tint = vec3(vUv.xy, 0.5);

    FragColor = uTint * vec4(tint.rgb, 1.0);
}