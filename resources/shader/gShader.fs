#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoord;
in vec3 ourColor;
in float brightness;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D ourTexture;

void main()
{
    if(texture(ourTexture, TexCoord).a < 0.5){
        discard;
    }
    gAlbedoSpec.rgb = texture(ourTexture, TexCoord).rgb * brightness;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(ourTexture, TexCoord).a;
    // store the fragment position vector in the first gbuffer texture
    gPosition.rgb = FragPos;
    gPosition.a = 1.0;
    // also store the per-fragment normals into the gbuffer
    gNormal.rgb = Normal;
    gNormal.a=1.0;
    // and the diffuse per-fragment color
    //gAlbedoSpec.rgb = texture(ourTexture, TexCoord).rgb * brightness;
    
}