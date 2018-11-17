#version 450
layout(local_size_x = 1, local_size_y = 1) in;	//local group of shaders
layout(rgba8, binding = 0) uniform image2D img_input; //input image
layout (std430, binding=0) volatile buffer shader_data
{
  ivec2 pixels[16384];
};

void main()
{
    // uint index = uint(gl_GlobalInvocationID.x);
    // ivec2 texcoords = pixels[index];
    // imageStore(img_input, texcoords, vec4(1,0,1,1));

    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    imageStore(img_input, pixel_coords, vec4(1, 1, 1, 1));
}
