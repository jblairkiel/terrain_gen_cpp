#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;

out vec4 FragColor;

uniform vec3 uLightDir = normalize(vec3(0.3, 1.0, 0.2));
uniform vec3 uColor;

void main()
{
    float diff = max(dot(normalize(vNormal), -uLightDir), 0.1);
    vec3 base = uColor;
    vec3 color = base * diff;
    FragColor = vec4(color, 1.0);
}