#pragma once
#include "VBO.h"

class VAO
{
	public:
		unsigned int ID{};
		VAO();
		void linkToVAO(int location, int size, VBO vbo) const;
        void linkToVAO(int location, int size, float stride, const void* pointer, VBO vbo) const;
		void bind() const;
		static void unbind();
		void deleteVAO() const;
};

