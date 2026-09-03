#version 450 core
out vec4 FragColor;

in vec2 vUv;

uniform vec4 uTint;
uniform sampler2D uBaseTexture;
uniform sampler2D uOverTexture;

void main() {
    vec3 tint = vec3(vUv.xy, 0.5);
    vec4 final = uTint * vec4(tint.rgb, 1.0) * texture(uBaseTexture, vUv);
    vec4 overlay = texture(uOverTexture, vUv * 4.0);

    final = mix(final, overlay, overlay.a * 0.5);
    FragColor = final;
}