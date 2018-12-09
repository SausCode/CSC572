#version 430

#define GHOST_TINT_PER_SAMPLE 1 // Apply txGhostGradientColor inside the sample loop instead of at the end.
#define DISABLE_CHROMATIC_ABERRATION 0 // Takes 3x fewer samples.

out vec4 color;
in vec2 fragTex;
in vec3 fragNor;
layout(location = 0) uniform sampler2D col_tex;
layout(location = 1) uniform sampler2D pos_tex;
layout(location = 2) uniform sampler2D norm_tex;
layout(location = 3) uniform sampler2D ghost_tex;


uniform int	uGhostCount;
uniform float uGhostSpacing;
uniform float uGhostThreshold;
uniform float uHaloRadius;
uniform float uHaloThickness;
uniform float uHaloThreshold;
uniform float uHaloAspectRatio;
uniform float uChromaticAberration;
uniform float uDownsample;
uniform float uGlobalBrightness;
uniform float uStarburstOffset;
uniform int debug_on;

vec3 ApplyThreshold(in vec3 _rgb, in float _threshold)
{
	return max(_rgb - vec3(_threshold), vec3(0.0));
}

vec3 SampleSceneColor(in vec2 _uv)
{
	#if DISABLE_CHROMATIC_ABERRATION
		return textureLod(col_tex, _uv, uDownsample).rgb;
	#else
		vec2 offset = normalize(vec2(0.5) - _uv) * uChromaticAberration;
		return vec3(
			textureLod(col_tex, _uv + offset, uDownsample).r,
			textureLod(col_tex, _uv, uDownsample).g,
			textureLod(col_tex, _uv - offset, uDownsample).b
		);
	#endif
}

vec3 SampleGhosts(in vec2 _uv, in float _threshold)
{
	vec3 ret = vec3(0.0);
	vec2 ghostVec = (vec2(0.5) - _uv) * uGhostSpacing;
	for (int i = 0; i < uGhostCount; ++i) {
		// sample scene color
		vec2 suv = fract(_uv + ghostVec * vec2(i));
		vec3 s = SampleSceneColor(suv);
		s = ApplyThreshold(s, _threshold);

		// tint/weight
		float distanceToCenter = distance(suv, vec2(0.5));
		#if GHOST_TINT_PER_SAMPLE
			s *= textureLod(pos_tex, vec2(distanceToCenter, 0.5), 0.0).rgb; // incorporate weight into tint gradient
		#else
			float weight = 1.0 - smoothstep(0.0, 0.75, distanceToCenter); // analytical weight
			s *= weight;
		#endif

		ret += s;
	}
	#if !GHOST_TINT_PER_SAMPLE
		ret *= textureLod(pos_tex, vec2(distance(_uv, vec2(0.5)), 0.5), 0.0).rgb;
	#endif

	return ret;
}

vec3 SampleHalo(in vec2 _uv, in float _radius, in float _aspectRatio, in float _threshold)
{
	vec2 haloVec = vec2(0.5) - _uv;
	haloVec.x /= _aspectRatio;
	haloVec = normalize(haloVec);
	haloVec.x *= _aspectRatio;
	vec2 wuv = (_uv - vec2(0.5, 0.0)) / vec2(_aspectRatio, 1.0) + vec2(0.5, 0.0);
	float haloWeight = distance(wuv, vec2(0.5));
	haloVec *= _radius;
	//haloWeight = Window_Cubic(haloWeight, _radius, uHaloThickness );
	//return SampleSceneColor(_uv + haloVec) * haloWeight;
	return ApplyThreshold(SampleSceneColor(_uv + haloVec), _threshold) * haloWeight;
}

float SampleStarburst() {
	// starburst
	vec2 centerVec = fragTex - vec2(0.5);
	float d = length(centerVec);
	float radial = acos(centerVec.x / d);
	float mask =
		texture(norm_tex, vec2(radial + uStarburstOffset * 1.0, 0.0)).r
		* texture(norm_tex, vec2(radial - uStarburstOffset * 0.5, 0.0)).r
		;
	mask = clamp(mask + (1.0 - smoothstep(0.0, 0.3, d)), 0.0, 1.0);

	// lens dirt
	//mask *= textureLod(txLensDirt, vUv, 0.0).r;

	//fResult = textureLod(txFeatures, vUv, 0.0).rgb;// *mask * uGlobalBrightness;
	return mask;
}

void main()
{
	color.a = 1;
	vec3 texturecolor = texture(col_tex, fragTex).rgb;
	vec3 normals = texture(norm_tex, fragTex).rgb;
	vec3 world_pos = texture(pos_tex, fragTex).rgb;
	vec3 ghost_col = texture(ghost_tex, fragTex).rgb;

	vec2 uv = vec2(1.0) - fragTex;
	if (debug_on == 0) {
		float starburst = SampleStarburst();
		color.rgb = texturecolor;
		color.rgb += SampleGhosts(uv, uGhostThreshold) * starburst;
		color.rgb += SampleHalo(uv, uHaloRadius, uHaloAspectRatio, uHaloThreshold) * starburst;
		
	}
	else if (debug_on == 1){
		color.rgb = vec3(0);
		color.rgb += SampleGhosts(uv, uGhostThreshold);
		color.rgb += SampleHalo(uv, uHaloRadius, uHaloAspectRatio, uHaloThreshold);
	}
	else if (debug_on == 2){
		color.rgb = vec3(0);
		color.rgb += SampleGhosts(uv, uGhostThreshold);		
	}
	else if (debug_on == 3){
		color.rgb = vec3(0);
		color.rgb += SampleHalo(uv, uHaloRadius, uHaloAspectRatio, uHaloThreshold);
	}
	else if (debug_on == 4){
		color.rgb = texturecolor;
	}
	else if (debug_on == 5) {
		color.rgb = vec3(SampleStarburst());
	}
	return;
}