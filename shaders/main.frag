#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec4 FragPosLightSpace;
} fs_in;

out vec4 FragColor;

uniform sampler2D shadowMap;

uniform vec3 lightDir = vec3(-0.3, -1.0, -0.2);
uniform vec3 baseColor = vec3(0.2, 0.7, 0.3);

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float bias = 0.005;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            float currentDepth = projCoords.z - bias;
            if (currentDepth > closestDepth)
                shadow += 1.0;
        }
    }

    shadow /= 9.0;
    return shadow;
}

void main()
{
    vec3 normal = normalize(fs_in.Normal);
    vec3 light = normalize(-lightDir);

    float diff = max(dot(normal, light), 0.0);

    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);

    vec3 color = baseColor * diff;
    color = mix(color, color * 0.3, shadow);

    FragColor = vec4(color, 1.0);
}