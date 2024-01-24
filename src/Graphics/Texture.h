#pragma once

#include<iostream>
#include<glad/glad.h>
#include "stb_image.h"


class Texture
{
	public:
	unsigned int ID;

	Texture(const char* fileLocation);

	void Bind();
	void Unbind();
	void Delete();

	private:
	unsigned char* data;
	int width;
	int height;
	int nrChannels;
};

