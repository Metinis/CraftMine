#version 330 core
in vec3 aPos;
in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;
out float brightness;
out vec4 FragPos;

void main()
{
    ourColor = vec3(0.5, 0.0, 0.0);
    TexCoord = aTexCoord;
    gl_Position = projection * model * view * vec4(aPos, 1.0);
}
