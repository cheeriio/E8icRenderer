#version 330 core

in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

in vec3 LightDirection_tangentspace;
in vec3 EyeDirection_tangentspace;

in vec3 FragPos;
in vec3 Normal;

out vec3 color;

uniform sampler2D DiffuseTextureSampler;
uniform sampler2D NormalTextureSampler;
uniform samplerCube DepthSampler;

uniform vec3 LightPosition;
uniform vec3 CameraPosition;
uniform float far_plane;

vec3 gridSamplingDisk[20] = vec3[](
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(vec3 fragPos) {
    vec3 fragToLight = FragPos - LightPosition;
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
	float LightPower = 20.0f;
	
	vec3 MaterialDiffuseColor = texture(DiffuseTextureSampler, UV).rgb;
	vec3 MaterialAmbientColor = vec3(0.5, 0.5, 0.5) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.3, 0.3, 0.3);

	vec3 TextureNormal_tangentspace = normalize(texture(NormalTextureSampler, UV).rgb * 2.0 - 1.0);
	
	float distance = length( LightPosition - Position_worldspace );

	vec3 n = TextureNormal_tangentspace;
	vec3 l = normalize(LightDirection_tangentspace);

	float cosTheta = clamp(normalize(dot(Normal_cameraspace, LightDirection_cameraspace)), 0, 1);

	vec3 E = normalize(EyeDirection_tangentspace);
	vec3 R = reflect(-l, n);

	vec3 halfway_vector = normalize(E + l);
    float spec = pow(max(dot(n, halfway_vector), 0.0), 225.0f);
	
    float shadow = ShadowCalculation(FragPos); 

	color = 
		MaterialAmbientColor + (1.0 - shadow) * 
        (MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance * distance) +
		MaterialSpecularColor * LightColor * LightPower * spec / (distance * distance));
}
