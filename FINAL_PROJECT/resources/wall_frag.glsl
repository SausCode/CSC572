#version 430 core

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 pos_out;
layout(location = 2) out vec4 norm_out;

in vec3 fragPos;
in vec2 fragTex;
in vec3 fragNor;
in vec4 fragViewPos;
in vec4 worldPos;

layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform sampler2D tex2;
layout(location = 2) uniform sampler2D tex3;

uniform vec4 uScale;
uniform vec4 uBias;
uniform vec2 txSize;
uniform int uSrcLevel;

void main()
{
	vec3 texturecolor = texture(tex, fragTex).rgb;
	vec3 normalfromtex = texture(tex2, fragTex).rgb;
	vec3 ghost_texture = texture(tex3, fragTex).rgb;
	vec3 texturenormal = (normalfromtex + vec3(1, 1, 1));
	texturenormal = texturenormal*0.5;
	vec3 ey = normalize(fragNor);
	vec3 ez = vec3(0,0,1);
	vec3 ex = cross(ez, ey);
	mat3 TBN = mat3(ex, ey, ez);
	vec3 readynormal = normalize(TBN*texturenormal);
	pos_out = worldPos;
	norm_out = vec4(readynormal, 1);

	pos_out.rgb = ghost_texture;
	pos_out.a = 1;

	color = vec4(0.0);
	// 3x3 Gaussian blur
	const float kernel[9] = {
		0.077847, 0.123317, 0.077847,
		0.123317, 0.195346, 0.123317,
		0.077847, 0.123317, 0.077847,
	};
	const vec2 scale = 2.0 / vec2(txSize); // scale can increase the blur radius 
	const vec2 offsets[9] = {
		vec2(-1.0, -1.0), vec2( 0.0, -1.0), vec2( 1.0, -1.0),
		vec2(-1.0,  0.0), vec2( 0.0,  0.0), vec2( 1.0,  0.0),
		vec2(-1.0,  1.0), vec2( 0.0,  1.0), vec2( 1.0,  1.0),
	};
	for (int i = 0; i < 9; ++i) {
		color += textureLod(tex, fragTex + offsets[i] * scale, uSrcLevel) * kernel[i];
	}
	color.a = 1;
}