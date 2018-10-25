#version 450 
#extension GL_ARB_shader_storage_buffer_object : require
layout(local_size_x = 1024, local_size_y = 1) in;	
layout (binding = 0, offset = 0) uniform atomic_uint ac;

//	for texture handling
layout(rgba8, binding = 0) uniform image2D img;


//local group of shaders
layout (std430, binding=0) volatile buffer shader_data
{ 
  vec4 dataA[1025];
  ivec4 dataB[1025];
};
uniform int sizeofbuffer;
uniform int odd;

void swap(uint index1, uint index2){
	barrier();
	vec4 temp = dataA[index1];
	dataA[index1] = dataA[index2];
	dataA[index2] = temp;
}

void main() 
{
	uint index = gl_LocalInvocationID.x;

	if(index >= sizeofbuffer-1){
		return;
	}
	else{
		if (odd == 1){
			if ((index % 2) == 1){
				// odd
				if (dataA[index].y > dataA[index+1].y){
					swap(index, index+1);
				}
			}
		}
		else{
			if ((index % 2) == 0){
				// even
				if (dataA[index].y > dataA[index+1].y){
					swap(index, index+1);
				}
			}		
		}
	}
}