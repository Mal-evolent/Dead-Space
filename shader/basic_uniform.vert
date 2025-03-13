#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColor;
layout (location = 2) in vec3 VertexNormal;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    Color = VertexColor;
    Normal = mat3(transpose(inverse(model))) * VertexNormal; // Transform normal to world space
    FragPos = vec3(model * vec4(VertexPosition, 1.0)); // Transform position to world space
    gl_Position = projection * view * model * vec4(VertexPosition, 1.0);
}
