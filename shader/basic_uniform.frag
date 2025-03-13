#version 460

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;
in vec3 LightPos;
in vec3 ViewPos;

layout (location = 0) out vec4 FragColor;

void main() {
    // Ambient
    vec3 ambient = 0.1 * Color;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * Color;

    // Specular
    vec3 viewDir = normalize(ViewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.5) * spec; // Assuming a white specular light

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
