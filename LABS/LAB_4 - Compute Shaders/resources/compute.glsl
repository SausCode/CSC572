#version 450 
#extension GL_ARB_shader_storage_buffer_object : require
layout(local_size_x = 1024, local_size_y = 1) in;	
layout (binding = 0, offset = 0) uniform atomic_uint ac;

//	for texture handling
layout(rgba8, binding = 0) uniform image2D img;


//local group of shaders
layout (std430, binding=0) volatile buffer shader_data
{ 
  vec4 dataA[4097];
  ivec4 dataB[4097];
};
uniform int sizeofbuffer;
uniform int odd;

void swap(uint index1, uint index2){
	//barrier();
	memoryBarrier();
	vec4 temp = dataA[index1];
	dataA[index1] = dataA[index2];
	dataA[index2] = temp;
}

void main() 
{
	//uint index = gl_LocalInvocationID.x;
	uint index = gl_GlobalInvocationID.x;

	int workpershaderunit = sizeofbuffer/1024;
	for (uint i = 0; i < workpershaderunit; i++){
		uint worknum = 1024*i + index;
		if (worknum >= sizeofbuffer){
			return;
		}

		if (odd == 1){
			if ((worknum % 2) == 1){
				// odd
				if (dataA[worknum].y > dataA[worknum+1].y){
					swap(worknum, worknum+1);
				}
			}
		}
		else{
			if ((worknum % 2) == 0){
				// even
				if (dataA[worknum].y > dataA[worknum+1].y){
					swap(worknum, worknum+1);
				}
			}
		}
	}




//	if(index >= sizeofbuffer-1){
//		return;
//	}
//	else{
//		if (odd == 1){
//			if ((index % 2) == 1){
//				// odd
//				if (dataA[index].y > dataA[index+1].y){
//					swap(index, index+1);
//				}
//			}
//		}
//		else{
//			if ((index % 2) == 0){
//				// even
//				if (dataA[index].y > dataA[index+1].y){
//					swap(index, index+1);
//				}
//			}		
//		}
//	}
}