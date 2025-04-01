#version 460

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D hdrBuffer;
uniform float exposure;
uniform float time;  // Add time uniform for animated noise

// Noise functions
float random(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

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

    // Add screen noise
    float noiseValue = noise(TexCoords * 100.0 + time * 0.1);
    float noiseIntensity = 0.02; // Adjust this to control noise strength
    vec3 noiseColor = vec3(noiseValue * noiseIntensity);
    
    // Add subtle color distortion in darker areas
    float darkness = 1.0 - length(hdrColor);
    vec3 colorNoise = vec3(
        random(TexCoords + time * 0.05),
        random(TexCoords + time * 0.05 + 1.0),
        random(TexCoords + time * 0.05 + 2.0)
    ) * darkness * 0.1;

    hdrColor += noiseColor + colorNoise;

    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    
    // Enhance darker areas
    mapped = pow(mapped, vec3(1.2));
    
    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
} 