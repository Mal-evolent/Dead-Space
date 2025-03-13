#version 460

layout (binding = 0) uniform samplerCube skybox;

in vec3 Vec;
layout(location = 0) out vec4 FragColor;

void main(){
    vec3 texColor = texture(skybox, normalize(Vec)).rgb;
    FragColor = vec4(texColor, 1.0);
}