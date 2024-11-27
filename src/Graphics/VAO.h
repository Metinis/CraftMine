#pragma once
#include "VBO.h"

class VAO
{
	public:
		unsigned int ID{};
		VAO();
		void LinkToVAO(int location, int size, VBO vbo) const;
        void LinkToVAO(int location, int size, float stride, const void* pointer, VBO vbo) const;
		void Bind() const;
		static void Unbind();
		void Delete() const;
};

