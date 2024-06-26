#include "Mesh.h"
void Mesh::setData(std::vector<glm::vec3> _vertices, std::vector<glm::vec3> _normals, std::vector<glm::vec2> _UVs, std::vector<GLuint> _indices, std::vector<float> _brightnessFloats)
{
    //std::lock_guard<std::mutex> lock(meshMutex);
    loadedData = false;
    vertices = _vertices;
    normals = _normals;
    UVs = _UVs;
    indices = _indices;
    brightnessFloats = _brightnessFloats;
}
void Mesh::clearData()
{
    std::lock_guard<std::mutex> lock(meshMutex);
    loadedData = false;
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
    if (meshNormalVBO != nullptr) {
        meshNormalVBO->Delete();
        delete meshNormalVBO;
        meshNormalVBO = nullptr;
    }
    deletedData = true;
}
void Mesh::render(Shader& _shader)
{
    std::lock_guard<std::mutex> lock(meshMutex);
    if(loadedData && meshVAO != nullptr && meshIBO != nullptr && meshUVVBO != nullptr && meshVBO != nullptr && meshBrightnessVBO != nullptr && meshNormalVBO != nullptr) {
        _shader.use();
        meshVAO->Bind();
        meshIBO->Bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
        meshVAO->Unbind();
        meshIBO->Unbind();
    }
}
void Mesh::loadData(Shader& _shader){
    //std::lock_guard<std::mutex> lock(meshMutex);
    loadedData = false;
    _shader.use();
    clearData();

    meshVAO = new VAO();
    meshVAO->Bind();

    meshVBO = new VBO(vertices);
    meshVBO->Bind();
    meshVAO->LinkToVAO(0, 3, *meshVBO);
    meshVBO->Unbind();

    meshUVVBO = new VBO(UVs);
    meshUVVBO->Bind();
    meshVAO->LinkToVAO(2, 2, *meshUVVBO);
    meshUVVBO->Unbind();

    meshNormalVBO = new VBO(normals);
    meshNormalVBO->Bind();
    meshVAO->LinkToVAO(1, 3, *meshNormalVBO);
    meshNormalVBO->Unbind();

    meshBrightnessVBO = new VBO(brightnessFloats);
    meshBrightnessVBO->Bind();
    meshVAO->LinkToVAO(_shader.getAttribLocation("aBrightness"), 1, *meshBrightnessVBO);
    meshBrightnessVBO->Unbind();
    meshIBO = new IBO(indices);
    // Unbind the VAO after setting up the main shader
    meshVAO->Unbind();


}

