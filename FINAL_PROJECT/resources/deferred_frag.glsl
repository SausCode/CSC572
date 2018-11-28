#version 430

out vec4 color;
in vec2 fragTex;
in vec3 fragNor;
layout(location = 0) uniform sampler2D col_tex;
layout(location = 1) uniform sampler2D pos_tex;
layout(location = 2) uniform sampler2D norm_tex;


uniform int	uGhostCount;
uniform float uGhostSpacing;
uniform float uGhostThreshold;

vec3 ApplyThreshold(in vec3 _rgb, in float _threshold)
{
	return max(_rgb - vec3(_threshold), vec3(0.0));
}

vec3 SampleSceneColor(in vec2 _uv)
{
	return textureLod(col_tex, _uv, 2).rgb;
}


vec3 SampleGhosts(in vec2 _uv, in float _threshold){
	vec3 ret = vec3(0.0);
	vec2 ghostVec = (vec2(0.5) - fragTex) * uGhostSpacing;
	for (int i = 0; i < uGhostCount; ++i) {
		vec2 suv = fract(_uv + ghostVec * vec2(i));
		float d = distance(suv, vec2(0.5));
		float weight = 1.0 - smoothstep(0.0, 0.75, d); // reduce contributions from samples at the screen edge
		vec3 s = SampleSceneColor(suv);
		s = ApplyThreshold(s, _threshold);
		ret += s * weight;
	}
	return ret;	
}

//vec3 SampleHalo(in vec2 _uv, in float _radius, in float _aspectRatio, in float _threshold)
//{
//	vec2 haloVec = vec2(0.5) - _uv;
//	#if DISABLE_HALO_ASPECT_RATIO
//		haloVec = normalize(haloVec);
//		float haloWeight = distance(_uv, vec2(0.5));
//	#else
//		haloVec.x /= _aspectRatio;
//		haloVec = normalize(haloVec);
//		haloVec.x *= _aspectRatio;
//		vec2 wuv = (_uv - vec2(0.5, 0.0)) / vec2(_aspectRatio, 1.0) + vec2(0.5, 0.0);
//		float haloWeight = distance(wuv, vec2(0.5));
//	#endif
//	haloVec *= _radius;
//	haloWeight = Window_Cubic(haloWeight, _radius, uHaloThickness );
//	return ApplyThreshold(SampleSceneColor(_uv + haloVec), _threshold) * haloWeight;
//}

void main()
{
	color.a = 1;
	vec3 texturecolor = texture(col_tex, fragTex).rgb;
	vec3 normals = texture(norm_tex, fragTex).rgb;
	vec3 world_pos = texture(pos_tex, fragTex).rgb;
	vec2 uv = vec2(1.0) - fragTex;
	color.rgb = SampleGhosts(uv, uGhostThreshold);
	return;
}