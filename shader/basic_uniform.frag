#version 460

// Input from vertex shader
in vec3 FragPos;      // World-space fragment position
in vec2 TexCoords;    // UV texture coordinates
in mat3 TBN;          // Tangent-Bitangent-Normal matrix for normal mapping

out vec4 FragColor;   // Final output color

// Material and scene uniforms
uniform vec3 lightPos;        // Light position in world space
uniform float lightRadius = 50.0;  // Default size if not specified
uniform float lightIntensity = 1.2; // Moderate intensity - between dark and bright
uniform vec3 viewPos;         // Camera position in world space
uniform sampler2D albedoMap;  // Base color texture
uniform sampler2D normalMap;  // Normal map for surface detail
uniform sampler2D metallicMap; // Metallic properties texture
uniform sampler2D roughnessMap; // Surface roughness texture
uniform sampler2D aoMap;      // Ambient occlusion texture
uniform samplerCube environmentMap; // Skybox texture for environment reflections
uniform float chromaticAberrationStrength = 0.05;

const float PI = 3.14159265359;

// Transform normal from tangent to world space
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
    // Using reference shader's normal mapping approach
    return normalize(TBN * tangentNormal);
}

// GGX/Trowbridge-Reitz NDF
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// Schlick-GGX geometry shadowing function
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// Smith's method combining shadowing and masking
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel-Schlick for direct lighting
vec3 fresnelSchlick(float cosTheta, vec3 F0, float roughness)
{
    vec3 scaledF0 = F0 * 0.5;
    return scaledF0 + (max(vec3(1.0 - roughness), scaledF0) - scaledF0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

//Fresnel-Schlick for environment reflections
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    vec3 scaledF0 = F0 * 0.3;
    return scaledF0 + (max(vec3(1.0 - roughness), scaledF0) - scaledF0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // Apply chromatic aberration by offsetting color channels
    vec2 texOffset = (TexCoords - 0.5) * 2.0; // Convert UVs to -1 to 1 range
    vec2 redOffset = TexCoords + texOffset * chromaticAberrationStrength;
    vec2 blueOffset = TexCoords - texOffset * chromaticAberrationStrength;
    
    // Sample colors with offset
    vec3 albedoR = pow(texture(albedoMap, redOffset).rgb, vec3(2.2));
    vec3 albedoG = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    vec3 albedoB = pow(texture(albedoMap, blueOffset).rgb, vec3(2.2));
    
    // Combine channels
    vec3 albedo = vec3(albedoR.r, albedoG.g, albedoB.b);
    // Using reference shader's exact albedo brightness
    albedo *= 1.0;

    // material parameter handling
    float metallic = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao = texture(aoMap, TexCoords).r;

    //normal mapping
    vec3 N = normalize(TBN[2]); // or use vec3 N = normalize(TBN * vec3(0, 0, 1));
    vec3 V = normalize(viewPos - FragPos);
    
    // F0 calculation
    vec3 F0 = mix(vec3(0.04), albedo, metallic * 0.7);

    // Direct lighting calculation
    vec3 L = normalize(lightPos - FragPos);
    vec3 H = normalize(V + L);
    
    // Calculate distance to light for attenuation (keeping from current shader)
    float distance = length(lightPos - FragPos);
    
    //BRDF values
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0, roughness);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    
    // specular scaling
    vec3 specular = numerator / denominator * 0.8;

    // kS and kD calculations
    vec3 kS = F * 0.8; 
    vec3 kD = (1.0 - kS) * (1.0 - metallic * 0.7);
    float NdotL = max(dot(N, L), 0.0);
    
    // Calculate light attenuation (keeping from current shader but applying reference mood)
    float attenuation = 1.0 / (1.0 + (distance * distance) / (lightRadius * lightRadius));
    
    // Apply light with attenuation and intensity from current shader
    vec3 Lo = (kD * albedo / PI + specular) * NdotL * attenuation * lightIntensity;

    // Environment reflection calculation using reference shader parameters
    vec3 R = reflect(-V, N);
    vec3 F_roughness = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS_env = F_roughness * 0.8; // Using exact reference value
    vec3 kD_env = (1.0 - kS_env) * (1.0 - metallic * 0.7); // Using exact reference value
    
    // environment reflection calculation
    vec3 envReflection = pow(texture(environmentMap, R).rgb, vec3(0.8));
    
    // environment BRDF scaling
    vec3 envBRDF = kD_env * albedo + kS_env * envReflection;

    // ambient lighting
    vec3 ambient = vec3(0.01) * albedo * ao;
    
    // Final color composition with reference shader's exact environment contribution
    vec3 color = ambient + Lo + envBRDF * 0.8;
    
    // tone mapping
    color = color / (color + vec3(0.8));
    
    // gamma correction
    color = pow(color, vec3(1.0/2.0));

    FragColor = vec4(color, 1.0);
}
