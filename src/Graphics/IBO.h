#pragma once
#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <vector>

class IBO
{
	public:
		unsigned int ID{};

		explicit IBO(const std::vector<GLuint>& data);
		void SetNewData(const std::vector<GLuint>& data) const;
		void Bind() const;
		static void Unbind();
		void Delete() const;
};

