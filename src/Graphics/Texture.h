#pragma once

#include<iostream>
#include<glad/glad.h>
#include "stb_image.h"


class Texture
{
	public:
	unsigned int ID{};

	explicit Texture(const char* fileLocation);

    void setTexture(const char* fileLocation);
	void Bind() const;
	static void Unbind();
	void Delete() const;

	private:
	unsigned char* data;
	int width{};
	int height{};
	int nrChannels{};
};

