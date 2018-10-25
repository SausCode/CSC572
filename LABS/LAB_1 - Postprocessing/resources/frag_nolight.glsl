#version 450 core 

out vec4 color;

in vec2 fragTex;

layout(location = 0) uniform sampler2D tex;

uniform vec2 blur_direction;

void main()
{
	vec3 texturecolor = texture(tex, fragTex, 0).rgb;

	float weights[] = {0.227027, 0.1945946, 0.1216216, 0.05054, 0.016216};
	float xp = 1./(640*2);
	float yp = 1./(480*2);

	if (blur_direction.y == 2){
		texturecolor.rgb = vec3(0,0,0);
	}

	else{
		if (blur_direction.x == 1){
			for (int i = -5; i<=5; i++){
				if (i==0){
					continue;
				}
				vec3 col = texture(tex, fragTex+vec2(i*xp,0),0).rgb;
				texturecolor += col * 1./abs(i); //weights[abs(i)-1];
			}
		}

		if (blur_direction.y == 1){
			for (int i = -5; i<=5; i++){
				if (i==0){
					continue;
				}
				vec3 col = texture(tex, fragTex+vec2(0,i*yp),0).rgb;
				texturecolor += col * 1./abs(i); //weights[abs(i)-1];
			}
		}
	}

	color.rgb = texturecolor;
	// No blur
	if (blur_direction.x == 5){
		// Debug mode get rid of blur
		color.rgb = texture(tex, fragTex, 0).rgb;
	}
	color.a=1;
}
