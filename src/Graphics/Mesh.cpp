#include "Mesh.h"
Mesh::Mesh(Shader& _shader) : shader(_shader)
{
    //shader = _shader;
}
void Mesh::setData(std::vector<glm::vec3> _vertices, std::vector<glm::vec2> _UVs, std::vector<GLuint> _indices)
{
    vertices = _vertices;
    UVs = _UVs;
    indices = _indices;
}
void Mesh::clearData()
{
    shader.use();
    if (meshVAO != nullptr) {
        meshVAO->Delete();
        delete meshVAO;
        meshVAO = nullptr;
    }

    if (meshVBO != nullptr) {
        meshVBO->Delete();
        delete meshVBO;
        meshVBO = nullptr;
    }

    if (meshUVVBO != nullptr) {
        meshUVVBO->Delete();
        delete meshUVVBO;
        meshUVVBO = nullptr;
    }

    if (meshIBO != nullptr) {
        meshIBO->Delete();
        delete meshIBO;
        meshIBO = nullptr;
    }
}
void Mesh::render()
{
    if(meshVAO != nullptr && meshIBO != nullptr && indices.size() > 0 && &shader != nullptr && meshUVVBO != nullptr && meshVBO != nullptr) {
        shader.use();
        meshVAO->Bind();
        meshIBO->Bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        meshVAO->Unbind();
        meshIBO->Unbind();
    }
}
void Mesh::loadData()
{
    shader.use();
    // Reset pointers to nullptr
    clearData();

    // Create new buffers and load data
    meshVAO = new VAO();
    meshVAO->Bind();

    meshVBO = new VBO(vertices);
    meshVBO->Bind();
    meshVAO->LinkToVAO(shader.getAttribLocation("aPos"), 3, *meshVBO);
    meshVBO->Unbind();

    meshUVVBO = new VBO(UVs);
    meshUVVBO->Bind();
    meshVAO->LinkToVAO(shader.getAttribLocation("aTexCoord"), 2, *meshUVVBO);
    meshUVVBO->Unbind();

    meshVAO->Unbind();

    meshIBO = new IBO(indices);
}
