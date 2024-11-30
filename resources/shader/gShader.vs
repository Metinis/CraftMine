#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 rgbiLight;    // RGBI light (color and brightness)

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;
out float brightness;
out vec3 FragPos;
out vec3 Normal;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    Normal = aNormal;
    ourColor = rgbiLight.rgb;
    TexCoord = aTexCoord;
    brightness = rgbiLight.a;
    FragPos = vec3(model * vec4(aPos, 1.0));
}

