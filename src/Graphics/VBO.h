#pragma once
#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <glm/vec2.hpp> // glm::vec2
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <vector>
#include <glm/glm.hpp>

class VBO
{
	public:
		unsigned int ID;
		VBO(const std::vector<glm::vec3>& data);

		// Constructor for Vector2 data
		VBO(const std::vector<glm::vec2>& data);
        //constructor for brightness
        VBO(const std::vector<float>& data);
		void SetNewData(const std::vector<glm::vec3>& data);
		void SetNewData(const std::vector<glm::vec2>& data);
		void Bind();
		void Unbind();
		void Delete();
};

