#version 430

out vec4 color;
in vec2 fragTex;
in vec3 fragNor;
layout(location = 0) uniform sampler2D ghost_tex;
layout(location = 1) uniform sampler2D halo_tex;
layout(location = 2) uniform sampler2D starburst_tex;
layout(location = 3) uniform sampler2D col_tex;

uniform vec2 txSize;
uniform int uSrcLevel;

void main()
{
	color.a = 1;
	//vec3 ghost_color = texture(ghost_tex, fragTex).rgb;
	//vec3 halo_color = texture(halo_tex, fragTex).rgb;
	//vec3 starburst_color = texture(starburst_tex, fragTex).rgb;
	vec3 texture_color = texture(col_tex, fragTex).rgb;

	vec3 ghost_color;
	vec3 halo_color;
	vec3 starburst_color;

	color = vec4(0.0);
	// 3x3 Gaussian blur
	const float kernel[9] = {
		0.077847, 0.123317, 0.077847,
		0.123317, 0.195346, 0.123317,
		0.077847, 0.123317, 0.077847,
	};
	const vec2 scale = 2.0 / vec2(txSize); // scale can increase the blur radius 
	const vec2 offsets[9] = {
		vec2(-1.0, -1.0), vec2(0.0, -1.0), vec2(1.0, -1.0),
		vec2(-1.0,  0.0), vec2(0.0,  0.0), vec2(1.0,  0.0),
		vec2(-1.0,  1.0), vec2(0.0,  1.0), vec2(1.0,  1.0),
	};
	for (int i = 0; i < 9; ++i) {
		float starburst = texture(starburst_tex, fragTex).r;
		ghost_color += textureLod(ghost_tex, fragTex + offsets[i] * scale, uSrcLevel).rgb * kernel[i];
		halo_color += textureLod(halo_tex, fragTex + offsets[i] * scale, uSrcLevel).rgb * kernel[i];
	}
	color.a = 1;
	float starburst = texture(starburst_tex, fragTex).r;
	ghost_color *= starburst;
	halo_color *= starburst;
	color.rgb = ghost_color + halo_color + texture_color;

	return;
}