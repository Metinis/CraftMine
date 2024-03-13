#pragma once
#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "VBO.h"

class VAO
{
	public:
		unsigned int ID;
		VAO();
		void LinkToVAO(int location, int size, VBO vbo);
        void LinkToVAO(int location, int size, float stride, void* pointer, VBO vbo);
		void Bind();
		void Unbind();
		void Delete();
};

