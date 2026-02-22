#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D fontAtlas;
uniform vec3 textColor;
uniform float alpha;

void main()
{
    float sampled = texture(fontAtlas, TexCoords).r;
    if (sampled < 0.01)
        discard;
    FragColor = vec4(textColor, sampled * alpha);
}
