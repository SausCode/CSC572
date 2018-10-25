#version 410 core
out vec4 color;

in vec2 frag_tex;
in float colgrad;

uniform sampler2D tex;

void main()
{
vec4 tcol = texture(tex, frag_tex);
color = tcol;
color.a = (color.r + color.g + color.b)/3.;
color.rgb *=colgrad;
}
