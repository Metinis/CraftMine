#version 330 core
in vec3 aPos;
in vec2 aTexCoord;
in vec3 aNormal;
in float aBrightness;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out vec3 ourColor;
out float brightness;
out vec3 FragPos;
out vec4 fragPosLightSpace;
out vec3 Normal;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    Normal = transpose(inverse(mat3(model))) * aNormal;
    ourColor = vec3(0.5, 0.0, 0.0);
    TexCoord = aTexCoord;
    brightness = aBrightness;
    FragPos = vec3(model * vec4(aPos, 1.0));
    fragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}
