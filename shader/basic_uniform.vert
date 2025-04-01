#version 460

// Vertex attributes
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColor;
layout (location = 2) in vec3 VertexNormal;
layout (location = 3) in vec2 VertexTexCoords;
layout (location = 4) in vec3 VertexTangent;
layout (location = 5) in vec3 VertexBitangent;

// Output to fragment shader
out vec3 Color;
out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out mat3 TBN;         // Tangent-Bitangent-Normal matrix

// Transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    // Transform position to world space
    FragPos = vec3(model * vec4(VertexPosition, 1.0));
    TexCoords = VertexTexCoords;
    
    // Construct TBN matrix for normal mapping
    vec3 T = normalize(mat3(model) * VertexTangent);
    vec3 B = normalize(mat3(model) * VertexBitangent);
    vec3 N = normalize(mat3(model) * VertexNormal);
    TBN = mat3(T, B, N);
    
    // Final position in clip space
    gl_Position = projection * view * model * vec4(VertexPosition, 1.0);
}
