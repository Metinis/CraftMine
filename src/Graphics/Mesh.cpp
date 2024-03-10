#include "Mesh.h"
Mesh::Mesh(Shader& _shader, Shader& _shadowMap) : shader(_shader), shadowMap(_shadowMap)
{
    //shader = _shader;
}
void Mesh::setData(std::vector<glm::vec3> _vertices, std::vector<glm::vec2> _UVs, std::vector<GLuint> _indices, std::vector<float> _brightnessFloats)
{
    vertices = _vertices;
    UVs = _UVs;
    indices = _indices;
    brightnessFloats = _brightnessFloats;
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
    if (meshBrightnessVBO != nullptr) {
        meshBrightnessVBO->Delete();
        delete meshBrightnessVBO;
        meshBrightnessVBO = nullptr;
    }
    /*if (meshFBO != nullptr) {
        meshFBO->Delete();
        delete meshFBO;
        meshFBO = nullptr;
    }*/
}
void Mesh::render()
{
    //glEnable(GL_DEPTH_TEST);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //shader.use();
    //renderShadowMap();
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.use();
    if(meshVAO != nullptr && meshIBO != nullptr && indices.size() > 0 && &shader != nullptr && meshUVVBO != nullptr && meshVBO != nullptr && meshBrightnessVBO != nullptr) {
        meshVAO->Bind();
        meshIBO->Bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
        meshVAO->Unbind();
        meshIBO->Unbind();
    }

}
void Mesh::loadData()
{
    // Load data for the main shader
    shader.use();
    clearData();

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

    meshBrightnessVBO = new VBO(brightnessFloats);
    meshBrightnessVBO->Bind();
    meshVAO->LinkToVAO(shader.getAttribLocation("aBrightness"), 1, *meshBrightnessVBO);
    meshBrightnessVBO->Unbind();

    meshIBO = new IBO(indices);

    // Unbind the VAO after setting up the main shader
    meshVAO->Unbind();

}

