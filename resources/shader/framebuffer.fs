#version 330 core
out vec4 FragColor;
  
in vec2 texCoords;

uniform sampler2D sampledTexture;
uniform float near_plane;
uniform float far_plane;
uniform bool inWater;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main()
{      
    if(inWater){
        vec4 sampledColor = texture(sampledTexture, texCoords);

        // Apply blue effect by increasing the blue component and reducing red and green
        float blueStrength = 0.2; // Adjust this value to control the intensity of the blue effect
        sampledColor.rgb = vec3(sampledColor.r * (1.0 - blueStrength), 
                            sampledColor.g * (1.0 - blueStrength), 
                            sampledColor.b + blueStrength * (1.0 - sampledColor.b));

        FragColor = sampledColor;
    } 
    else{
        FragColor = texture(sampledTexture, texCoords);
    }
}
