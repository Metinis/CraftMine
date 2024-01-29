#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
in float brightness;

float minBrightness = 0.1f;

uniform sampler2D ourTexture;


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


    // Set the output color
    FragColor = vec4(adjustedColor, sampledColor.a);
}

