#version 450 core
/*
	Shader for .ZMD2 skinned & morphing mesh
*/
in vec2 vUv;
//varying vec3 vNormal;
//varying vec4 vColour;
//varying vec4 vPos;
out vec4 FragColor;

uniform sampler2D uAlbedo;
uniform sampler2D uAmbientOcclusion;

void main() {
	//vec4 final = texture(uAlbedo, vUv) * texture(uAmbientOcclusion, vUv);

    vec4 final = texture(uAlbedo, vUv);
    FragColor = final;
}
