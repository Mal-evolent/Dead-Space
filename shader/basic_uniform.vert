#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColor;
layout (location = 2) in vec3 VertexNormal;
layout (location = 3) in vec2 VertexTexCoords;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    Normal = mat3(transpose(inverse(model))) * VertexNormal;
    FragPos = vec3(model * vec4(VertexPosition, 1.0));
    TexCoords = VertexTexCoords; // Pass the texture coordinates to the fragment shader
    gl_Position = projection * view * model * vec4(VertexPosition, 1.0);
}
