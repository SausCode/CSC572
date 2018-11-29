#version 450 core 
layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform sampler2D normalMap;
layout(location = 2) uniform sampler2D shadowMapTex;

in vec3 fragPos;
in vec2 fragTex;
in vec3 fragTan;
in vec3 fragBi;
in vec3 fragNor;


layout(location = 0) out vec4 color;
layout(location = 1) out vec4 outnormal;
layout(location = 2) out vec4 outpos;
layout(location = 3) out vec4 outtan;


uniform mat4 lightSpace;
void main()
{
	vec3 texturecolor = texture(tex, fragTex).rgb;
	vec3 normalcolor = texture(normalMap, fragTex).rgb;
	normalcolor = normalize((normalcolor - vec3(0.5,0.5,0.5))*2.0);		//re-calculate real 3D normal	
	//normal mapping
	float normalmapfactor=0.1;
	vec3 tangent,normal, binormal;
	tangent = normalize(fragTan);
	normal = normalize(fragNor);
	binormal = normalize(fragBi);
	
	binormal = cross(normal,tangent);
    mat3 TBN = mat3(tangent, binormal, normal);
	vec3 bumpNormal = TBN * normalcolor; //rotate normal into tangent space
	
	bumpNormal = normalize(bumpNormal);
	bumpNormal = normalize(bumpNormal*normalmapfactor + normal*(1.-normalmapfactor));
	//renormalize the tangent
	binormal = cross(bumpNormal,tangent);
	tangent = cross(binormal, bumpNormal);
	//onto textures
	outnormal = vec4(bumpNormal,1);
	outpos =  vec4(fragPos,1);
	outtan = vec4(tangent,1);
	color.rgb = texturecolor;
	color.a = 1.0f;
}
