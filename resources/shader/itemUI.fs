#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
in float brightness;

uniform sampler2D ourTexture;


void main()
{
    vec4 sampledColor = texture(ourTexture, TexCoord);
    //sampledColor.rgb *= brightness;
    vec3 adjustedColor = sampledColor.rgb * brightness;

    // Set the output color
    FragColor = vec4(adjustedColor, sampledColor.a);
}

