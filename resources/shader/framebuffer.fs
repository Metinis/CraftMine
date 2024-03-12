#version 330 core
out vec4 FragColor;
  
in vec2 texCoords;

uniform sampler2D sampledTexture;


void main()
{             
    float depthValue = texture(sampledTexture, texCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}
