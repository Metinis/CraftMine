#include "VAO.h"

VAO::VAO()
{
	glGenVertexArrays(1, &ID);
	bind();
}
void VAO::linkToVAO(const int location, const int size, VBO vbo) const
{
	bind();
	vbo.bind();
    glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, 0, nullptr);
	unbind();
}
void VAO::linkToVAO(const int location, const int size, const float stride, const void* pointer, VBO vbo) const
{
    bind();
    vbo.bind();
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, static_cast<int>(stride), pointer);
    unbind();
}
void VAO::bind() const
{
	glBindVertexArray(ID);
}
void VAO::unbind()
{
	glBindVertexArray(0);
}
void VAO::deleteVAO() const
{
	glDeleteVertexArrays(1, &ID);
}
