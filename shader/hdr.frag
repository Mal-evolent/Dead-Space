#version 460

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D hdrBuffer;
uniform float exposure;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
  
    // Add cold, dark color tint
    vec3 coldTint = vec3(0.8, 0.9, 1.0); // Slight blue tint
    hdrColor *= coldTint;

    // Add vignette effect
    vec2 center = vec2(0.5, 0.5);
    float dist = length(TexCoords - center);
    float vignette = smoothstep(0.5, 1.5, dist);
    hdrColor *= 1.0 - vignette * 0.5;

    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    
    // Enhance darker areas
    mapped = pow(mapped, vec3(1.2));
    
    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
} 