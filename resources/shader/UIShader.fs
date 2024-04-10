#version 330 core


uniform sampler2D texture1; // Texture sampler
in vec2 TexCoords;
out vec4 FragColor;

void main() {
    //FragColor = vec4(1.0, 1.0, 1.0, 0.75); // White color for the crosshair
    //FragColor = vec4(TexCoords, 0.0, 1.0);
    FragColor = texture(texture1, TexCoords);
}
