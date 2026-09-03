#version 450 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;

out vec2 vUv;

uniform mat4 uMatTf;

void main() {
    gl_Position = uMatTf * vec4(inPos.xyz, 1.0);
    vUv = inUv;
}