#version 330 core

in vec2 aTexCoord;
in vec2 aPos;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoords = aTexCoord;
}
