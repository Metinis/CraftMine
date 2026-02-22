#include "Texture.h"

Texture::Texture(const char* fileLocation)
{

	data = stbi_load(fileLocation, &width, &height, &nrChannels, 0);

	glGenTextures(1, &ID);
	bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);

	unbind();
}
void Texture::setTexture(const char* fileLocation)
{
    data = stbi_load(fileLocation, &width, &height, &nrChannels, 0);
    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
}
void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, ID);
}
void Texture::unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture::deleteTex() const
{
	glDeleteTextures(1, &ID);
}
