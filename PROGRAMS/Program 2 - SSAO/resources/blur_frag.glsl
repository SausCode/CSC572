#version 450 core 

out vec4 color;

in vec2 fragTex;

layout(location = 0) uniform sampler2D ambient_occlusion;
layout(location = 1) uniform sampler2D gPosition;
layout(location = 2) uniform sampler2D gNormal;
layout(location = 3) uniform sampler2D gColor;

uniform vec3 light_color;
uniform vec3 light_pos;

void main()
{
	/* Used from example - https://learnopengl.com/Advanced-Lighting/SSAO*/

    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, fragTex).rgb;
    vec3 Normal = texture(gNormal, fragTex).rgb;
    vec3 Diffuse = texture(gColor, fragTex).rgb;
    float AmbientOcclusion = texture(ambient_occlusion, fragTex).r;
    
    // blinn-phong (in view-space)
    vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion); // here we add occlusion factor
    vec3 lighting  = ambient; 
    vec3 viewDir  = normalize(-FragPos); // viewpos is (0.0.0) in view-space
    // diffuse
    vec3 lightDir = normalize(light_pos - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light_color;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light_color * spec;
    // attenuation
    float dist = length(light_pos - FragPos);
	float attenuation = .1;
    diffuse  *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;

    vec4 FragColor = vec4(lighting, 1.0);

    vec2 texelSize = 1.0 / vec2(textureSize(ambient_occlusion, 0));
    vec3 result = vec3(0);
	float size = 0;
    for (int x = -3; x < 3; ++x) 
    {
        for (int y = -3; y < 3; ++y) 
        {
			size++;
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ambient_occlusion, fragTex + offset).xyz;
        }
    }
	color.xyz = result/(size);
	color.a = 1;
	color.xyz = FragColor.xyz * color.xyz;
}
