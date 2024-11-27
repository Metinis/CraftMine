#include "IBO.h"

IBO::IBO(const std::vector<GLuint>& data)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long>(data.size() * sizeof(GLsizei)), data.data(), GL_STATIC_DRAW);
}
void IBO::SetNewData(const std::vector<GLuint>& data) const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long>(data.size() * sizeof(GLsizei)), data.data(), GL_STATIC_DRAW);
}
void IBO::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}
void IBO::Unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void IBO::Delete() const
{
	glDeleteBuffers(1, &ID);
}