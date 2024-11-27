#include "VAO.h"

VAO::VAO()
{
	glGenVertexArrays(1, &ID);
	Bind();
}
void VAO::LinkToVAO(const int location, const int size, VBO vbo) const
{
	Bind();
	vbo.Bind();
    glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, 0, nullptr);
	Unbind();
}
void VAO::LinkToVAO(const int location, const int size, const float stride, const void* pointer, VBO vbo) const
{
    Bind();
    vbo.Bind();
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, static_cast<int>(stride), pointer);
    Unbind();
}
void VAO::Bind() const
{
	glBindVertexArray(ID);
}
void VAO::Unbind()
{
	glBindVertexArray(0);
}
void VAO::Delete() const
{
	glDeleteVertexArrays(1, &ID);
}
