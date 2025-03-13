#version 460

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;
in vec3 TexCoords;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;
uniform samplerCube skybox;

vec3 calculateBlinnPhong(vec3 normal, vec3 fragPos, vec3 viewPos, vec3 lightPos, vec3 ambientColor, vec3 diffuseColor, vec3 specularColor, float shininess, vec3 color)
{
    // Ambient
    vec3 ambient = ambientColor * color;

    // Diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseColor * diff * color;

    // Specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularColor * spec;

    return ambient + diffuse + specular;
}

void main()
{
    // Blinn-Phong result
    vec3 blinnPhongResult = calculateBlinnPhong(Normal, FragPos, viewPos, lightPos, ambientColor, diffuseColor, specularColor, shininess, Color);

    // Skybox color
    vec3 skyboxColor = texture(skybox, TexCoords).rgb;

    // Blend Blinn-Phong result with skybox color
    vec3 result = mix(skyboxColor, blinnPhongResult, 0.5);

    FragColor = vec4(result, 1.0);
}
