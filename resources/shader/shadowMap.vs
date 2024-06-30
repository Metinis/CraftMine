#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 lightProjection;
uniform mat4 model;

out vec2 TexCoord;

void main()
{
    TexCoord = aTexCoord;
    gl_Position = lightProjection * model * vec4(aPos, 1.0);
}
