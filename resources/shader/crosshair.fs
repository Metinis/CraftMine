#version 330 core


uniform sampler2D texture1; // Texture sampler
in vec2 TexCoords;
out vec4 FragColor;

void main() {
    vec4 texColor = texture(texture1, TexCoords);

    //if (texColor.a > 0.0) {
        // Invert RGB channels
    //    texColor.rgb = 1.0 - texColor.rgb;
    //}

    FragColor = texColor;
}
