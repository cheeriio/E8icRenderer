#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;
in vec3 Normal;

uniform samplerCube DepthSampler;

uniform vec3 Color;
uniform vec3 LightPosition;
uniform float far_plane;

vec3 gridSamplingDisk[20] = vec3[](
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(vec3 fragPos) {
    vec3 fragToLight = Position_worldspace - LightPosition;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.05; 
    float samples = 4.0;
    float offset = 0.1;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    {
        for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        {
            for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            {
                float closestDepth = texture(DepthSampler, fragToLight + vec3(x, y, z)).r;
                closestDepth *= far_plane;
                if(currentDepth - bias > closestDepth)
                    shadow += 1.0;
            }
        }
    }
    shadow /= (samples * samples * samples);
    return shadow;
}

void main() {
    vec3 LightColor = vec3(1, 1, 1);
	float LightPower = 10.0f;

    vec3 MaterialDiffuseColor = Color;
	vec3 MaterialAmbientColor = vec3(0.3, 0.3, 0.3) * Color;
	vec3 MaterialSpecularColor = vec3(0.2, 0.2, 0.2);

    float distance = length( LightPosition - Position_worldspace );

    float cosTheta = clamp(normalize(dot(Normal, LightPosition - Position_worldspace)), 0, 1);

    vec3 n = normalize(Normal_cameraspace);
	vec3 E = normalize(EyeDirection_cameraspace);
    vec3 l = normalize(LightDirection_cameraspace);
	vec3 R = reflect(-l, n);

	vec3 halfway_vector = normalize(E + l);
    float spec = pow(max(dot(n, halfway_vector), 0.0), 400.0f);

    float shadow = ShadowCalculation(Position_worldspace); 

    vec3 result_color =
		MaterialAmbientColor + (1.0 - shadow) * 
        (MaterialDiffuseColor * LightColor * LightPower / (distance * distance));
		// MaterialSpecularColor * LightColor * LightPower * spec / (distance * distance));

	FragColor = vec4(result_color, 1.0);
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
