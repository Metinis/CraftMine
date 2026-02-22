#version 330 core
out vec4 FragColor;
uniform vec3 playerColor;
void main() {
    FragColor = vec4(playerColor, 1.0);
}
