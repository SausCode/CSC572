#version 450 
layout(local_size_x = 1, local_size_y = 1) in;											//local group of shaders
//layout(rgba16f, binding = 0) uniform image3D img_source;									//input image
layout(binding = 0) uniform sampler3D src;
layout(rgba16f, binding = 1) uniform image3D dst;									//input image
layout(r32ui, binding = 2) uniform uimage3D src_count;									//input image
uniform int actualsize;
uniform int integer_textures_on;
//layout(binding = 1) uniform sampler3D occupancy;
const int SRC_LEVEL = 0;
void main() 
	{

	ivec3 threadId = ivec3(gl_GlobalInvocationID.xyz);	

	vec3 srcSize = textureSize(src, SRC_LEVEL).xyz;
	ivec3 dstSize = imageSize(dst);
	vec3 tc = vec3(threadId) / vec3(dstSize);
    vec3 texelSize = 1.0 / srcSize;
    tc += 0.5 * texelSize;
	
	if(integer_textures_on!=0 && actualsize==1)
		if(imageLoad(src_count, threadId).x==0)
			return;
//	 if (textureLod(src, tc, SRC_LEVEL).a < 0.01) 
  //      return;
    

	const ivec3 offsets[] = ivec3[](
        ivec3(0, 0, 0), ivec3(0, 0, 1), ivec3(0, 1, 0), ivec3(0, 1, 1),
        ivec3(1, 0, 0), ivec3(1, 0, 1), ivec3(1, 1, 0), ivec3(1, 1, 1)
    );

    vec4 value = vec4(0);
    for (int i = 0; i < 8; i++) {
        value += textureLodOffset(src, tc, SRC_LEVEL, offsets[i]);
    }
    value *= 0.125;
//	col = vec4(1,0,0,1);
	imageStore(dst, threadId,value);
	}