#version 450
layout(local_size_x = 1, local_size_y = 1) in;	//local group of shaders
layout(rgba8, binding = 0) uniform image2D img_input; //input image
layout(rgba8, binding = 1) uniform image2D tex_input; //input image
layout (std430, binding=0) volatile buffer shader_data
{
  ivec2 pixels[16384];
};

void main()
{
    uint index = uint(gl_GlobalInvocationID.x);
    ivec2 texcoords = pixels[index];
	vec4 color = vec4(0, 0, 0, 1);
	for (int i = -5; i <= 5; i++) {
		if (i == 0) {
			continue;
		}
		vec3 col = imageLoad(tex_input, texcoords + ivec2(i, 0)).rgb;
		if (!(col.r < .1 && col.g < .1 && col.b < .1)) {
			color.rgb += col * 1. / abs(i);
		}
	}
	/*vec4 tex_color = imageLoad(tex_input, texcoords);*/
    //imageStore(img_input, texcoords, color);
	imageStore(tex_input, texcoords, color);
}
