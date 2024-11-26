#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
in float aBrightness;

out vec2 TexCoord;
out float brightness;
out vec3 FragPos;
out vec3 Normal;
out vec4 FragPosLightSpace[16];

layout(std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16]; // UBO for 16 light space matrices
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;     // Time variable for animating waves

void main()
{
    // Create wave effect by modifying aPos.y using a sine wave
    float waveHeight = 0.05; // Amplitude of waves
    float waveSpeed = 0.75;  // Speed of wave movement
    vec3 animatedPos = aPos;
    animatedPos.y += ((sin(aPos.x * 5.0 + time * waveSpeed) + 1) * waveHeight + (sin(aPos.z * 5.0 + time * waveSpeed) + 1) * waveHeight);

    gl_Position = projection * view * model * vec4(animatedPos, 1.0);

    // Normal correction for animated positions
    //Normal = normalize(transpose(inverse(mat3(model))) * aNormal);
    //Normal = normalize(transpose(inverse(mat3(model))) * vec3(0.0, 1.0, 0.0));
    Normal = aNormal;

    TexCoord = aTexCoord;
    brightness = aBrightness;
    FragPos = vec3(model * vec4(animatedPos, 1.0));
    //FragPos = aPos;

}

