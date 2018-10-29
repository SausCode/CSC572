#version 450 core
in vec2 fragTex;
layout(location = 0) uniform sampler2D gPosition;
layout(location = 1) uniform sampler2D gNormal;
layout(location = 2) uniform sampler2D gAlbedoSpec;
layout(location = 3) uniform sampler2D noiseTexture;

layout(location = 0) out vec4 color;

uniform vec3 samples[64];

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(640*3/4.0, 480*3/4.0); // screen = 1280x720

int kernelSize = 64;
float radius = 0.25;
float bias = 0.025;

in mat4 projection;

void main()
{
	/* Used from example - https://learnopengl.com/Advanced-Lighting/SSAO*/

    // get input for SSAO algorithm
    vec3 fragPos = texture(gPosition, fragTex).xyz;
    vec3 normal = normalize(texture(gNormal, fragTex).rgb);
	vec3 albedo = texture(gAlbedoSpec, fragTex).xyz;
    vec3 randomVec = normalize(texture(noiseTexture, fragTex * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sampleVec = TBN * samples[i]; // from tangent to view-space
        sampleVec = fragPos + sampleVec * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sampleVec, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        if (sampleDepth >= sampleVec.z + bias) {
			occlusion += 1.0 * rangeCheck;
		} else {
			occlusion += 0.0 * rangeCheck;
		}
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    color.rgb = vec3(occlusion);
	color.a = 1;
}
