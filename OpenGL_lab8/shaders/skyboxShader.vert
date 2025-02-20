#version 410 core

layout (location = 0) in vec3 vertexPosition;
out vec3 textureCoordinates;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    mat4 viewNoTranslation = mat4(mat3(view)); // Eliminăm translația
    gl_Position = projection * viewNoTranslation * vec4(vertexPosition, 1.0);
    textureCoordinates = vertexPosition;
}
