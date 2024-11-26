#version 410 core

layout(location = 0) out float FragDepth;

uniform float alphaThreshold = 0.1;
uniform sampler2D ourTexture;
in vec2 FragTexCoord;

void main(){

    vec4 texColor = texture(ourTexture, FragTexCoord);

    if (texColor.a < alphaThreshold)
    {
        discard;
    }

    FragDepth = gl_FragCoord.z;
}