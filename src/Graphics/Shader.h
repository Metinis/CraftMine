#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
class Shader
{
public:
	//The ID of the shader
	unsigned int ID;

	//Constructor to build shader
	Shader(const char* vertexPath, const char* fragmentPath);

	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);

	//use/activate shader
	void use() const;

	//uniform functions in the shader path, accesses the values in shader files.

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, glm::vec3 value) const;
	void setMat4(const std::string& name, glm::mat4 value) const;

    GLint getAttribLocation(const std::string& name) const;
};

#endif

