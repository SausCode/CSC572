#version 450 core 

layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform sampler2D texnorm;
layout(location = 2) uniform sampler2D texpos;
layout(location = 3) uniform sampler2D textan;
layout(location = 4) uniform sampler2D shadowMapTex;
//layout(binding = 2,rgba16f) uniform image3D vox_output;
layout(binding = 5) uniform sampler3D vox_output0;
layout(binding = 6) uniform sampler3D vox_output1;
layout(binding = 7) uniform sampler3D vox_output2;
layout(binding = 8) uniform sampler3D vox_output3;
layout(binding = 9) uniform sampler3D vox_output4;
layout(binding = 10) uniform sampler3D vox_output5;
layout(binding = 11) uniform sampler3D vox_output6;

out vec4 color;

in vec2 fragTex;

uniform vec3 campos;
uniform vec3 lightpos;
uniform vec3 lightdir;
uniform mat4 lightSpace;

uniform float steps_for_tracing;
uniform float cut_off_alpha;
uniform float reflective_cone_angle;
uniform float ambient_factor;
uniform float shadow_factor;

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
    const ivec2 offsets[numSamples] = ivec2[](
        ivec2(0, 0), ivec2(1, 0), ivec2(0, 1), ivec2(-1, 0), ivec2(0, -1)
    );

    for (int i = 0; i < numSamples; i++) {
        if (fragDepth > textureOffset(shadowMapTex, shifted.xy, offsets[i]).r) {
            shadowFactor += 1;
        }
    }
    shadowFactor /= numSamples;

    return shadowFactor;
}

vec3 voxel_transform(vec3 pos)
	{
	//sponza is in the frame of [-10,10], and we have to map that to [0,255] in x, y, z
	vec3 texpos= pos + vec3(10,10,10);	//[0,20]
	texpos /= 20.;				//[0,1]
	//pos*=255;				//[0,255]
	//ivec3 ipos = ivec3(pos);
	return texpos;
	}
vec4 getmimappedcolor(vec3 texposition,uint mip)
{
if(mip ==0) return texture(vox_output0, texposition);
else if(mip == 1) return texture(vox_output1, texposition);
else if(mip == 2) return texture(vox_output2, texposition);
else if(mip == 3) return texture(vox_output3, texposition);
else if(mip == 4) return texture(vox_output4, texposition);
else if(mip == 5) return texture(vox_output5, texposition);
else			  return texture(vox_output6, texposition);
return vec4(0,0,0,0);
}
vec4 sampling(vec3 texposition,float mipmap)
{
uint imip = uint(mipmap);
float linint = mipmap - float(imip);
vec4 colA = getmimappedcolor(texposition,0);
vec4 colB = getmimappedcolor(texposition,imip+1);
return colA * (1-linint) + colB*linint;
}

vec3 cone_tracing(vec3 conedirection,vec3 pixelpos,float coneHalfAngle,float stepfactor,float break_condition)
{
conedirection=normalize(conedirection);
float voxelSize = 20./256.;				//[-10,10] / resolution	
//pixelpos += conedirection*voxelSize;	//to get some distance to the pixel against self-inducing
vec4 trace = vec4(0);
float distanceFromConeOrigin = voxelSize*2;
float dec[]={1,0.9,0.85,0.8,0.75,0.7,0.65,0.6,0.55,0.5,0.45,0.4,0.35,0.3,0.25,0.2,0.15,0.1,0.05};
for(int i=0;i<10;i++)
	{
	float coneDiameter = 2 * tan(coneHalfAngle) * distanceFromConeOrigin;
	float mip = log2(coneDiameter / voxelSize);
	pixelpos = pixelpos + conedirection*distanceFromConeOrigin;
	vec3 texpos = voxel_transform(pixelpos);
	trace +=sampling(texpos,mip) * dec[i];
	//return trace.rgb;
	if(trace.a>break_condition)break;
	distanceFromConeOrigin+=voxelSize*stepfactor;
	}
return trace.rgb;
}
uniform float manualmipmaplevel;

void main()
{
	
	//		----------------------		gathering all necessary pixel data			----------------------
	vec3 texturecolor = texture(tex, fragTex).rgb;
	vec3 normal = texture(texnorm, fragTex).rgb;

	vec3 tangent = texture(textan, fragTex).rgb;
	vec3 binorm = cross(tangent,normal);
	vec3 worldpos = texture(texpos, fragTex).rgb;
	
	//		----------------------		shadow mapping			----------------------
	vec4 fragLightSpacePos = lightSpace * vec4(worldpos,1);
	float shadowFactor = 1.0 - calcShadowFactor(fragLightSpacePos);
	float ambienceshadowFactor = 0.1 + shadowFactor*0.9;

	//		----------------------		diffuse lighting			----------------------
	vec3 lightDir = -lightdir;//normalize(lightPos - fragPos);
	vec3 lightColor = vec3(1.0);
	float light = dot(lightDir, normal);	
	vec3 diffuseColor = clamp(light, 0.0f, 1.0f) * lightColor;

	//		----------------------		specular lighting			----------------------
	
	vec3 camvec = normalize(-campos - worldpos);
	vec3 h = normalize(camvec + lightDir);
	float spec = pow(dot(h, normal), 50);
	vec3 specColor = clamp(spec, 0.0f, 1.0f) * lightColor ;

	//		----------------------		tracing in normal direction			----------------------
	
	vec3 texpos = voxel_transform(worldpos);
	
	float coneHalfAngle = 0.571239; //27 degree
	vec3 voxelcolor = cone_tracing(normal,worldpos,coneHalfAngle,1,1);
	voxelcolor += cone_tracing((normal + tangent) / 2, worldpos, coneHalfAngle, steps_for_tracing, cut_off_alpha);
	voxelcolor += cone_tracing((normal - tangent) / 2, worldpos, coneHalfAngle, steps_for_tracing, cut_off_alpha);
	voxelcolor += cone_tracing((normal + binorm) / 2, worldpos, coneHalfAngle, steps_for_tracing, cut_off_alpha);
	voxelcolor += cone_tracing((normal - binorm) / 2, worldpos, coneHalfAngle, steps_for_tracing, cut_off_alpha);
	voxelcolor += cone_tracing(reflect(-camvec, normal), worldpos, reflective_cone_angle, steps_for_tracing, cut_off_alpha);
	voxelcolor /= 6;

	color.rgb = ((diffuseColor + specColor)*ambienceshadowFactor + voxelcolor) * texturecolor;
	color.a = 1.0;
}
