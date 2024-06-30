#version 330 core

layout(location = 0) out float FragDepth;

in vec2 TexCoord;
uniform sampler2D ourTexture;
uniform float alphaThreshold = 0.5;

void main(){

    vec4 texColor = texture(ourTexture, TexCoord);

    // Perform alpha test
    if (texColor.a < alphaThreshold)
    {
        discard; // Discard transparent fragments
    }

    // Record the depth value
    FragDepth = gl_FragCoord.z;
}
