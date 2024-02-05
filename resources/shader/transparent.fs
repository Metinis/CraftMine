#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
in float brightness;

float minBrightness = 0.5f;

in vec4 FragPos;
uniform sampler2D ourTexture;
uniform vec3 cameraPos;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;


void main()
{
    vec4 sampledColor = texture(ourTexture, TexCoord);

    // Adjust brightness
    vec3 adjustedColor;
    if(brightness < minBrightness)
    {
        adjustedColor = sampledColor.rgb * minBrightness;
    }
    else
    {
        adjustedColor = sampledColor.rgb * brightness;
    }

    float distance = length(FragPos.xyz - cameraPos);

    float fogFactor = smoothstep(fogStart, fogEnd, distance);

    vec3 finalColor = mix(adjustedColor, fogColor, fogFactor);

    // Set the output color
    FragColor = vec4(finalColor, sampledColor.a);
}

