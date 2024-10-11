#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
in float aBrightness;

out vec2 TexCoord;
out vec3 ourColor;
out float brightness;
out vec3 FragPos;
out vec3 Normal;
out vec3 ViewDir;
out vec3 LightDir;
out vec4 ClipSpacePos;

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
    Normal = normalize(transpose(inverse(mat3(model))) * aNormal);
    //Normal = normalize(transpose(inverse(mat3(model))) * vec3(0.0, 1.0, 0.0));
    //Normal = aNormal;

    ourColor = vec3(0.0, 0.3, 0.8); // Water blue tint
    TexCoord = aTexCoord;
    brightness = aBrightness;
    FragPos = vec3(model * vec4(animatedPos, 1.0));
}

