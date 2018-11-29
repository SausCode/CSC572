#version 450 
layout(local_size_x = 1, local_size_y = 1) in;												//local group of shaders
//layout(rgba16f, binding = 0) uniform image3D img_source;									//input image
layout(rgba16f, binding = 0) uniform image3D dst;									//input image
layout(r32ui, binding = 1) uniform uimage3D src_r;									//input image
layout(r32ui, binding = 2) uniform uimage3D src_g;									//input image
layout(r32ui, binding = 3) uniform uimage3D src_b;									//input image
layout(r32ui, binding = 4) uniform uimage3D src_count;									//input image
//layout(binding = 1) uniform sampler3D occupancy;
const int SRC_LEVEL = 0;
void main() 
	{

	ivec3 threadId = ivec3(gl_GlobalInvocationID.xyz);	
	uint count	= imageLoad(src_count,threadId).x;
	if(count<1)return;
	uint red	= imageLoad(src_r,threadId).x;
	uint green	= imageLoad(src_g,threadId).x;
	uint blue	= imageLoad(src_b,threadId).x;
	
	float occupied = 0;
	if(red + green + blue > 0)
		occupied=1;
	red /=count;
	green /=count;
	blue /=count;

	vec4 value = vec4(float(red) / 255.,float(green) / 255.,float(blue) / 255.,occupied);
	imageStore(dst, threadId,value);
	}