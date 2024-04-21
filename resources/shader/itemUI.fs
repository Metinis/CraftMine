#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;


void main()
{
    vec4 sampledColor = texture(ourTexture, TexCoord);

    // Set the output color
    FragColor = sampledColor;
}

