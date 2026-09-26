#version 450 core
out vec4 FragColor;
in vec2 vUv;

uniform sampler2D uAlbedo;

void main() {
    vec4 final = texture(uAlbedo, vUv);
    
    FragColor = final;
    //FragColor = vec4(1.0);
}