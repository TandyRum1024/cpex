#version 450 core
/*
	Shader for .ZMD2 skinned & morphing mesh
*/
/*
	// Standard model data (xyz,uv,nxyz(f32),RGBA(u8))
	vertex_format_add_position_3d();
	vertex_format_add_texcoord();
	vertex_format_add_normal();
	vertex_format_add_color();
	// ZMD2: Part index (index (f32))
	vertex_format_add_custom(vertex_type_float1, vertex_usage_texcoord);
	// ZMD2: Bones (indices(f32 x 4),bone weights(f32 x 4))
	// Why float32s for bone indices instead of unsigned bytes...? GM's GLSL ES breaks it
	vertex_format_add_custom(vertex_type_float4, vertex_usage_texcoord);
	vertex_format_add_custom(vertex_type_float4, vertex_usage_texcoord);
*/
layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_Uv;
layout (location = 2) in vec3 in_Normal;
layout (location = 3) in vec4 in_Colour;
layout (location = 4) in float in_PartIdx;
layout (location = 5) in vec4 in_BoneIdx; // attribute ivec4 in_BoneIdx; -> we can't use ivec4
layout (location = 6) in vec4 in_BoneWeight;
/*
	// ZMD2: Morphs (morph1 xyz offset(f32 x 3),morph1 nxyz offset(f32 x 3),morph2 xyz offset(f32 x 3),morph2 nxyz offset(f32 x 3))
	vertex_format_add_custom(vertex_type_float3, vertex_usage_texcoord);
	vertex_format_add_custom(vertex_type_float3, vertex_usage_texcoord);
	vertex_format_add_custom(vertex_type_float3, vertex_usage_texcoord);
	vertex_format_add_custom(vertex_type_float3, vertex_usage_texcoord);
*/
// If everything else has been added with vertex_format_add_custom(**, vertex_usage_texcoord); we should be able to do this
layout (location = 7) in vec3 in_Morph1PosOff;
layout (location = 8) in vec3 in_Morph1NormalOff;
layout (location = 9) in vec3 in_Morph2PosOff;
layout (location = 10) in vec3 in_Morph2NormalOff;

out vec2 vUv;
//varying vec3 vNormal;
//varying vec4 vColour;
//varying vec4 vPos;
//varying float vDepth;

//uniform float uMorphWeights[MORPHS_MAX_GLOBAL];

// Matrices
//uniform mat4 uMatBones[BONES_MAX_GLOBAL];
//uniform mat4 uMatBonesNormal[BONES_MAX_GLOBAL];
//uniform mat4 uMatNormal;

// Parts visibility
//uniform float uPartVis[PARTS_MAX_GLOBAL];

// Tint
//uniform vec4 uTint;

// TEST
//uniform vec3 uZ3dCameraZNearFar;

//uniform mat4 uMatSmapDirectionals[SMAP_DIRECTIONALS_MAX];
//uniform vec3 uSmapDirectionalDir[SMAP_DIRECTIONALS_MAX];
//uniform float uSmapDirectionalOffsetScale[SMAP_DIRECTIONALS_MAX];

//mat4 matModel = gm_Matrices[MATRIX_WORLD];
//mat4 matViewProjection = gm_Matrices[MATRIX_PROJECTION] * gm_Matrices[MATRIX_VIEW];

uniform mat4 uMatModel;
uniform mat4 uMatView;
uniform mat4 uMatProjection;

mat4 matViewProjection = uMatProjection * uMatView;

/*
vec3 getSmapNormalOffset(float ndotl, vec3 normal, float offsetScale) {
	// https://therealmjp.github.io/posts/shadow-maps/#biasing
	// Offset more when normal is perpendicular ("facing away") towards light direction. hence the `(1.0 - ndotl)`
	return normal * clamp(1.0 - ndotl, 0.0, 1.0) * offsetScale;
}
*/

void main() {
	vec4 vertPos = vec4(in_Position, 1.0);
	vec4 vertNormal = vec4(in_Normal, 0.0);

	vec4 posModel = uMatModel * vec4(vertPos.xyz, 1.0);
	gl_Position = matViewProjection * posModel;
	//vertNormalSum = uMatNormal * vertNormalSum;
		
	//vNormal = normalize(vertNormalSum.xyz);
	//vNormal = vertNormal;
	vUv = in_Uv;
	//vColour = in_Colour;
	//vPos = posModel;
	/*
	int partIdx = int(in_PartIdx);
	bool partVisible = int(uPartVis[partIdx]) == 1;
	
	if (!partVisible) {
		// (invisible part. skip expensive processing and just shove vertices into a point behind camera)
		// hopefully doing this will make the GPU do nothing about it in Fragment shader
		gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
		vNormal = in_Normal;
		vUv = in_Uv;
		vColour = in_Colour;
	}
	else {
		int morphIdx1 = partIdx * MORPHS_MAX_PER_VERT;
		int morphIdx2 = partIdx * MORPHS_MAX_PER_VERT + 1;
		
		// 1] Apply morph
		float morphWeight1 = uMorphWeights[morphIdx1];
		float morphWeight2 = uMorphWeights[morphIdx2];
		
		vec4 vertPos = vec4(in_Position + (in_Morph1PosOff * morphWeight1) + (in_Morph2PosOff * morphWeight2), 1.0);
	    vec4 vertNormal = vec4(in_Normal + (in_Morph1NormalOff * morphWeight1) + (in_Morph2NormalOff * morphWeight2), 0.0);
		
		// 2] Apply bone transform by weighted sum
		vec4 vertPosSum = vec4(0.0);
		vec4 vertNormalSum = vec4(0.0);
		float weightSum = in_BoneWeight[0] + in_BoneWeight[1] + in_BoneWeight[2] + in_BoneWeight[3];
		
		for (int i=0; i<BONES_MAX_PER_VERT; i++) {
			float boneWeight = in_BoneWeight[i];
			int boneIdx = int(in_BoneIdx[i]);
			
			if (boneWeight <= 0.0 || boneIdx < 0) break;
			
			mat4 matBoneCurrent = uMatBones[boneIdx];
			mat4 matNormalCurrent = uMatBonesNormal[boneIdx];
			vec4 vertTransformed = matBoneCurrent * vertPos;
			vec4 normalTransformed = matNormalCurrent * vertNormal;
			
			vertPosSum += vertTransformed * boneWeight;
			// ..how tf do I do normals now..
			vertNormalSum += normalTransformed * boneWeight;
		}
		
		// Since the weighted sum starts at 0, if there is NO weights at all or weights under 1, the models vertices would be interpolated to (0, 0, 0)
		// We don't want that. So if weight sum is under 1, blend in the "original position" for the remaining weight
		//float weightSum = in_BoneWeight[0] + in_BoneWeight[1] + in_BoneWeight[2] + in_BoneWeight[3];
		//float restWeight = 1.0 - weightSum;
		float restWeight = 1.0 - weightSum;
		if (restWeight > 0.0) {
			vertPosSum += vertPos * restWeight;
			vertNormalSum += vertNormal * restWeight;
		}
		
		// Normalize
		//vertPosSum /= weightSum;
		
		// 3] Apply global transform
		vec4 posModel = vec4((matModel * vertPosSum).xyz, 1.0);
		gl_Position = matViewProjection * posModel;
		vertNormalSum = uMatNormal * vertNormalSum;
		
		vNormal = normalize(vertNormalSum.xyz);
		vUv = in_Uv;
		vColour = uTint * in_Colour;
		vPos = posModel;
		
		for (int i=0; i<SMAP_DIRECTIONALS_MAX; i++) {
			vec3 lightDir = normalize(-uSmapDirectionalDir[i]);
			float nDotL = dot(lightDir, vNormal);
			
			vSmapDirectionalPos[i] = uMatSmapDirectionals[i] * (posModel + vec4(getSmapNormalOffset(nDotL, vNormal, uSmapDirectionalOffsetScale[i]), 0.0));
			vSmapDirectionalDir[i] = lightDir;
		}
	}
	*/
}
