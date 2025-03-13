#version 460 

layout (location = 0) in vec3 VertexPosition;
out vec3 Vec;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    Vec = VertexPosition;
    mat4 viewNoTranslation = mat4(mat3(view));
    gl_Position = projection * viewNoTranslation * vec4(VertexPosition, 1.0);
    gl_Position.w = gl_Position.z;
}
