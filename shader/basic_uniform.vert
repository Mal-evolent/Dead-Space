#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColor;
layout (location = 2) in vec3 VertexNormal;
layout (location = 3) in vec2 VertexTexCoords;
layout (location = 4) in vec3 VertexTangent;
layout (location = 5) in vec3 VertexBitangent;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    FragPos = vec3(model * vec4(VertexPosition, 1.0));
    TexCoords = VertexTexCoords;
    
    vec3 T = normalize(mat3(model) * VertexTangent);
    vec3 B = normalize(mat3(model) * VertexBitangent);
    vec3 N = normalize(mat3(model) * VertexNormal);
    TBN = mat3(T, B, N);
    
    gl_Position = projection * view * model * vec4(VertexPosition, 1.0);
}
