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
	void bind() const;
	static void unbind();
	void deleteTex() const;

	private:
	unsigned char* data;
	int width{};
	int height{};
	int nrChannels{};
};

