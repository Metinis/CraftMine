#version 330 core
out vec4 FragColor;
  
in vec2 texCoords;

uniform sampler2D sampledTexture;


void main()
{             
    FragColor =  texture(sampledTexture, texCoords);
    //FragColor = vec4(depthValue);
}
