#version 330 core
in vec3 aPos;
in vec2 aTexCoord;
in float aBrightness;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;
out float brightness;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = vec3(0.5, 0.0, 0.0);
    TexCoord = aTexCoord;
    brightness = aBrightness;
}
