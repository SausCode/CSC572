#version 450
layout(local_size_x = 1, local_size_y = 1) in;	//local group of shaders
layout(rgba8, binding = 0) uniform image2D img_input; //input image
layout (std430, binding=0) volatile buffer shader_data
{
  ivec2 pixels[16384];
};

void main()
{
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 mask_color = imageLoad(img_input, pixel_coords);

    if (mask_color.r > .5){
    	int index = atomicAdd(pixels[16383].x, 1);
    	if (index >= 16382){
            pixels[16383].x = 0;
    		return;
    	}
    	pixels[index] = pixel_coords;
    }
     //imageStore(img_input, pixel_coords,vec4(1,1,0,1));
}
