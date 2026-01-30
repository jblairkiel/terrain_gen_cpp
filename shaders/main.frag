#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;
in vec4 vLightSpacePos;

out vec4 FragColor;

uniform sampler2D shadowMap;
uniform vec3 lightDir;

float calculateShadow(vec4 lightSpacePos)
{
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float shadow = 0.0;
    float bias = 0.002;
    float texelSize = 1.0 / 2048.0;

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float pcfDepth = texture(shadowMap,
                                     projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;
    return shadow;
}

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 light = normalize(-lightDir);

    float diff = max(dot(normal, light), 0.0);

    float shadow = calculateShadow(vLightSpacePos);

    vec3 baseColor = vec3(0.35, 0.55, 0.25);
    vec3 lighting = baseColor * (0.2 + diff * (1.0 - shadow));

    FragColor = vec4(lighting, 1.0);
}