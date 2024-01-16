#include "VAO.h"

VAO::VAO()
{
	glGenVertexArrays(1, &ID);
	Bind();
}
void VAO::LinkToVAO(int location, int size, VBO vbo)
{
	Bind();
	vbo.Bind();
	glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(location);
	Unbind();
}
void VAO::Bind()
{
	glBindVertexArray(ID);
}
void VAO::Unbind()
{
	glBindVertexArray(0);
}
void VAO::Delete()
{
	glDeleteVertexArrays(1, &ID);
}
