#version 460

// Vertex attributes
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColor;
layout (location = 2) in vec3 VertexNormal;
layout (location = 3) in vec2 VertexTexCoords;
layout (location = 4) in vec3 VertexTangent;
layout (location = 5) in vec3 VertexBitangent;

// Output to fragment shader
out vec3 FragPos;      // World-space position
out vec2 TexCoords;    // UV texture coordinates
out mat3 TBN;          // Tangent-Bitangent-Normal matrix

// Transformation matrices
uniform mat4 model;         // Model matrix
uniform mat4 view;          // View matrix
uniform mat4 projection;    // Projection matrix
uniform mat3 normalMatrix;  // Inverse transpose of the model matrix

void main()
{
    // Transform vertex position to world space
    FragPos = vec3(model * vec4(VertexPosition, 1.0));
    TexCoords = VertexTexCoords;

    // Construct TBN matrix for normal mapping
    vec3 T = normalize(normalMatrix * VertexTangent);
    vec3 B = normalize(normalMatrix * VertexBitangent);
    vec3 N = normalize(normalMatrix * VertexNormal);
    TBN = mat3(T, B, N);

    // Transform to clip space
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
