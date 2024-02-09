#version 330 core

out vec4 FragColor;

void main() {
    //vec4 previousColor = texelFetch(previousFramebuffer, ivec2(gl_FragCoord.xy), 0);
    FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White color for the crosshair
    //FragColor = vec4(1 - previousColor.x, 1 - previousColor.y, 1 - previousColor.z, 1.0);
}