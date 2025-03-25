#version 460

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;
uniform sampler2D spaceshipTexture;

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
    // Sample the spaceship texture
    vec3 texColor = texture(spaceshipTexture, TexCoords).rgb;

    // Blinn-Phong result using the texture color
    vec3 blinnPhongResult = calculateBlinnPhong(Normal, FragPos, viewPos, lightPos, ambientColor, diffuseColor, specularColor, shininess, texColor);

    FragColor = vec4(blinnPhongResult, 1.0);
}
