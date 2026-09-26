#version 450 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;

out vec2 vUv;

uniform mat4 uMatModel;
uniform mat4 uMatView;
uniform mat4 uMatProjection;

void main() {
    vec4 vertPos = vec4(inPos.xyz, 1.0);
    vec4 vertPosClip = uMatProjection * uMatView * uMatModel * vertPos;
    //vec4 vertPosClip = vertPos;

    gl_Position = vertPosClip;
    vUv = inUv;
}