#version 330 core
layout(location = 0) in vec3 vertPos;
/*layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;*/

uniform mat4 P;
uniform mat4 V;
uniform mat4 Vdir;
uniform mat4 M;
uniform float lorentz;
uniform vec3 camdir;
uniform vec3 campos;
out vec3 vertex_pos;
//out vec3 vertex_normal;
//out vec2 vertex_tex;

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
	
	vec4 tpos =  M * vec4(vertPos, 1.0);

	tpos= special_relativity_pos(tpos);

	vertex_pos = tpos.xyz;
	tpos = Vdir * V * tpos;

	gl_Position = P *tpos;

}
