#version 460

// Input from vertex shader
in vec3 FragPos;      // World-space fragment position
in vec2 TexCoords;    // UV texture coordinates
in mat3 TBN;          // Tangent-Bitangent-Normal matrix for normal mapping

out vec4 FragColor;   // Final output color

// Material and scene uniforms
uniform vec3 lightPos;        // Light position in world space
uniform float lightIntensity; // Light brightness
uniform vec3 viewPos;         // Camera position in world space
uniform sampler2D albedoMap;  // Base color texture
uniform sampler2D normalMap;  // Normal map for surface detail

const float PI = 3.14159265359;

void main()
{
    // Spherical UV mapping
    vec3 nrmPos = normalize(FragPos);
    float u = 0.5 + atan(nrmPos.z, nrmPos.x) / (2.0 * PI);
    float v = 0.5 - asin(nrmPos.y) / PI;
    vec2 sphericalUV = vec2(u, v);
    
    // Sample the albedo texture with spherical UVs
    vec3 albedo = texture(albedoMap, sphericalUV).rgb;
    
    // Get normal from normal map using spherical UVs
    vec3 tangentNormal = texture(normalMap, sphericalUV).xyz * 2.0 - 1.0;
    vec3 N = normalize(TBN * tangentNormal);
    
    // Calculate lighting direction and distance
    vec3 L = normalize(lightPos - FragPos);
    float distance = length(lightPos - FragPos);
    
    // Apply distance attenuation to light
    // Adjust the constants to control falloff rate
    float attenuation = 1.0 / (1.0 + 0.0002 * distance + 0.00000005 * distance * distance);
    
    // Basic ambient component
    vec3 ambient = albedo * 0.3;
    
    // Simple diffuse lighting with attenuation
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * albedo * lightIntensity * attenuation;
    
    // Combine lighting components
    vec3 result = ambient + diffuse;
    
    // Basic gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}
