#version 430 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

out vec2 fragTex;
out vec2 flippedFragTex;
out vec3 fragNor;

void main()
{
	gl_Position = vertPos;
	fragTex = vertTex;
	flippedFragTex = vertTex * vec2(1, -1);
	fragNor = (vec4(vertNor, 0.0)).xyz;
}
