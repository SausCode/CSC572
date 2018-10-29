#version 450 core 

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

in vec3 fragPos;
in vec2 fragTex;
in vec3 fragNor;
in vec4 fragViewPos;

layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform sampler2D tex2;


void main()
{
    // store the fragment position vector in the first gbuffer texture
    gPosition = fragViewPos;
    // also store the per-fragNor normals into the gbuffer
	gNormal.a = 1;
    gNormal.xyz = normalize(fragNor).xyz;
    // and the diffuse per-fragment color
    //gAlbedoSpec = vec4(0.95, 0.95, 0.95, 1);
	gAlbedoSpec.a = 1;
	gAlbedoSpec.xyz = texture(tex, fragTex).xyz;
}
