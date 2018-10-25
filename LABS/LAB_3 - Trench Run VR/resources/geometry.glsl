#version 450 core

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;



uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 Vdir;
uniform float lorentz;
uniform vec3 camdir;
uniform vec3 campos;

out vec2 frag_tex;
out float colgrad;

vec4 special_relativity_pos(vec4 worldpos)
{
vec3 ntovertex = normalize(worldpos.xyz - campos);
vec3 ncamdir = normalize(camdir);
float d = dot(ncamdir,ntovertex);
d = abs(d);
vec4 Vdirpos = V * worldpos;
float az = Vdirpos.z;
float nz = az;
if(nz>0)
	nz/=lorentz;
else
	nz*=lorentz;
float rz = mix(az,nz,d);
Vdirpos.z = rz;
mat4 Vdir_i = inverse(V);
worldpos = Vdir_i * Vdirpos;
return worldpos;
}

void main()
{


	vec4 midpos = gl_in[0].gl_Position;
	float z = midpos.z;
	
	midpos= special_relativity_pos(midpos);


	vec4 eyeoffset = vec4(0.3,0,0,0);

	midpos =   Vdir * V  * midpos;
	vec4 pos;
	float fact = 0.0008;
	float distancefact = 0;
	colgrad = 1. /pow(abs(midpos.z),0.1);
	distancefact = pow(abs(midpos.z),0.1);
	distancefact = midpos.z;
	fact *= distancefact;
	fact = 0.08 * lorentz;
	pos = midpos + vec4(-1,-1,0,0)*fact;
	frag_tex=vec2(0,0);
	gl_Position = (P * pos) + eyeoffset;
	EmitVertex();
	pos = midpos + vec4(1,-1,0,0)*fact;
	frag_tex=vec2(1,0);
	gl_Position = (P * pos) + eyeoffset;
	EmitVertex();
	pos = midpos + vec4(-1,1,0,0)*fact;
	frag_tex=vec2(0,1);
	gl_Position = (P * pos) + eyeoffset;
	EmitVertex();
	pos = midpos + vec4(1,-1,0,0)*fact;
	frag_tex=vec2(1,0);
	gl_Position = (P * pos) + eyeoffset;
	EmitVertex();
	pos = midpos + vec4(1,1,0,0)*fact;
	frag_tex=vec2(1,1);
	gl_Position = (P * pos) + eyeoffset;
	EmitVertex();
	pos = midpos + vec4(-1,1,0,0)*fact;
	frag_tex=vec2(0,1);
	gl_Position = (P * pos) + eyeoffset;
	EmitVertex();
    EndPrimitive();
}