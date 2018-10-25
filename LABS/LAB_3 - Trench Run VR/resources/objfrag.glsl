#version 330 core
out vec4 color;
//in vec3 vertex_normal;
in vec3 vertex_pos;
//in vec2 vertex_tex;
uniform vec3 addcolor;

uniform sampler2D tex;
//uniform sampler2D tex2;

void main()
{
//vec4 tcol = texture(tex, vertex_tex);
//color = tcol;
color = vec4(1,1,1,1);
color.rgb = addcolor;
}
