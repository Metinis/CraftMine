#include "VBO.h"

VBO::VBO(const std::vector<glm::vec4>& data)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, static_cast<long>(data.size() * sizeof(glm::vec4)), data.data(), GL_STATIC_DRAW);
}

VBO::VBO(const std::vector<glm::vec3>& data)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, static_cast<long>(data.size() * sizeof(glm::vec3)), data.data(), GL_STATIC_DRAW);
}

// Constructor for Vector2 data
VBO::VBO(const std::vector<glm::vec2>& data)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, static_cast<long>(data.size() * sizeof(glm::vec2)), data.data(), GL_STATIC_DRAW);
}
VBO::VBO(const std::vector<float>& data)
{
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, static_cast<long>(data.size() * sizeof(float)), data.data(), GL_STATIC_DRAW);
}
void VBO::SetNewData(const std::vector<glm::vec3>& data) const
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, static_cast<long>(data.size() * sizeof(glm::vec3)), data.data(), GL_STATIC_DRAW);
}
void VBO::SetNewData(const std::vector<glm::vec2>& data) const
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, static_cast<long>(data.size() * sizeof(glm::vec2)), data.data(), GL_STATIC_DRAW);
}
void VBO::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}
void VBO::Unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void VBO::Delete() const
{
	glDeleteBuffers(1, &ID);
}
