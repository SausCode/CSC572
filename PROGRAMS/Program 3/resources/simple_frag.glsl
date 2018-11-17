#version 450 core 
layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform sampler2D shadowMapTex;

in vec3 fragPos;
in vec2 fragTex;
in vec3 fragNor;
in vec4 fragLightSpacePos;

uniform vec3 campos;
uniform vec3 lightpos;
uniform vec3 lightdir;

uniform float sphereon;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 mask;

// Evaluates how shadowed a point is using PCF with 5 samples
// Credit: Sam Freed - https://github.com/sfreed141/vct/blob/master/shaders/phong.frag
float calcShadowFactor(vec4 lightSpacePosition) {
    vec3 shifted = (lightSpacePosition.xyz / lightSpacePosition.w + 1.0) * 0.5;

    float shadowFactor = 0;
    float bias = 0.01;
    float fragDepth = shifted.z - bias;

    if (fragDepth > 1.0) {
        return 0.0;
    }

    const int numSamples = 5;
    const ivec2 offset0 = ivec2(0, 0);
    const ivec2 offset1 = ivec2(1, 0);
    const ivec2 offset2 = ivec2(0, 1);
    const ivec2 offset3 = ivec2(-1, 0);
    const ivec2 offset4 = ivec2(0, -1);

    for (int i = 0; i < numSamples; i++) {
    	float tOff = 0.0;
    	if (i == 0){
    		tOff = textureOffset(shadowMapTex, shifted.xy, offset0).r;
    	}
    	else if (i == 1){
    		tOff = textureOffset(shadowMapTex, shifted.xy, offset1).r;
    	}
    	else if (i == 2){
    		tOff = textureOffset(shadowMapTex, shifted.xy, offset2).r;
    	}
    	else if (i == 3){
    		tOff = textureOffset(shadowMapTex, shifted.xy, offset3).r;
    	}
    	else if (i == 4){
    		tOff = textureOffset(shadowMapTex, shifted.xy, offset4).r;
    	}    	    	


        if (fragDepth > tOff) {
            shadowFactor += 1;
        }
    }
    shadowFactor /= numSamples;

    return shadowFactor;
}

void main()
{
	vec3 texturecolor = texture(tex, fragTex).rgb;
	float shadowFactor = 1.0 - calcShadowFactor(fragLightSpacePos);
	float ambienceshadowFactor = 0.1 + shadowFactor*0.9;

	//diffuse light
	//vec3 lightPos = lightpos;
	vec3 lightDir = -lightdir;//normalize(lightPos - fragPos);
	vec3 lightColor = vec3(1.0);

	vec3 normal = normalize(fragNor);

	float light = dot(lightDir, normal);	
	vec3 diffuseColor = clamp(light, 0.0f, 1.0f) * lightColor * ambienceshadowFactor;

	//specular light
	vec3 camvec = normalize(campos - fragPos);
	vec3 h = normalize(camvec + lightDir);

	float spec = pow(dot(h, normal), 500);

	vec3 specColor = clamp(spec, 0.0f, 1.0f) * lightColor * shadowFactor;

	color.rgb = (diffuseColor + specColor) * texturecolor;
	//color.rgb = vec3(shadowFactor);
	color.a = 1.0f;

	mask = vec4(sphereon,0,0,1);

	if (sphereon > .5) {
		color.rgb = vec3(0);
	}
}
