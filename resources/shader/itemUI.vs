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
out vec4 FragPos;

void main()
{
    ourColor = vec3(0.5, 0.0, 0.0);
    TexCoord = aTexCoord;
    brightness = aBrightness;
    gl_Position = model * vec4(aPos, 1.0);
}
