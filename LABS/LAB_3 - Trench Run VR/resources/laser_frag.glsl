#version 330 core 
in vec3 fragNor;
out vec4 color;
in vec3 pos;
in vec2 fragTex;


uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 offset;

void main()
{
	vec3 normal = normalize(fragNor);
	
	vec3 lp = vec3(100,100,100);
	vec3 ld = normalize(lp - pos);
	float light = dot(ld,normal);
	vec2 texcoord = fragTex + offset/100.;
	color = texture(tex2, texcoord*50.);// *texture(tex2, texcoord);// *light; //[TWOTEXTURES]

	float dist = length(pos);
	float s = dist/50.0;
	s = pow(s,2)-.2;
	color.r = 0.0;
	color.g = 0.0;
	color.b = 1.0;
	color.a=1-s;
	
}
