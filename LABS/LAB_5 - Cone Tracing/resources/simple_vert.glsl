#version  450 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in vec3 vertTan;
layout(location = 4) in vec3 vertBinorm;

uniform mat4 P;
uniform mat4 V;
uniform mat4 lightSpace;
uniform mat4 M;

out vec3 fragPos;
out vec3 fragTan;
out vec3 fragNor;
out vec2 fragTex;
out vec3 fragBi;


void main()
{
	fragPos = (M * vec4(vertPos, 1.0)).xyz;
	fragNor = (M * vec4(vertNor, 0.0)).xyz;
	fragTan = (M * vec4(vertTan, 0.0)).xyz;

	fragBi = (M * vec4(vertBinorm, 0.0)).xyz;

//	fragBi = cross(fragNor,fragTan);
//	fragTan = cross(fragBi,fragNor);

	fragTex = vertTex;
	gl_Position = P * V * M * vec4(vertPos, 1.0);
}
