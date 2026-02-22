#include "Mesh.h"

#include <utility>
void Mesh::setData(std::vector<glm::vec3> _vertices, std::vector<glm::vec3> _normals, std::vector<glm::vec2> UVs_, std::vector<GLuint> _indices, std::vector<float> _brightnessFloats)
{
    std::lock_guard<std::mutex> lock(meshMutex);
    vertices = std::move(_vertices);
    normals = std::move(_normals);
    UVs = std::move(UVs_);
    indices = std::move(_indices);
    brightnessFloats = std::move(_brightnessFloats);
}
bool Mesh::clearData()
{
        loadedData = false;
        if (meshVAO != nullptr) {
            meshVAO->deleteVAO();
            delete meshVAO;
            meshVAO = nullptr;
        }
        else{
           // std::cout<<"mesh mem leak \n";
           return false;
        }

        if (meshVBO != nullptr) {
            meshVBO->deleteVBO();
            delete meshVBO;
            meshVBO = nullptr;
        }
        else{
           // std::cout<<"mesh mem leak \n";

            return false;
        }
        if (meshUVVBO != nullptr) {
            meshUVVBO->deleteVBO();
            delete meshUVVBO;
            meshUVVBO = nullptr;
        }
        else{
          //  std::cout<<"mesh mem leak \n";

            return false;
        }
        if (meshIBO != nullptr) {
            meshIBO->deleteIBO();
            delete meshIBO;
            meshIBO = nullptr;
        }
        else{
          //  std::cout<<"mesh mem leak \n";

            return false;
        }
        if (meshBrightnessVBO != nullptr) {
            meshBrightnessVBO->deleteVBO();
            delete meshBrightnessVBO;
            meshBrightnessVBO = nullptr;
        }
        else{
          //  std::cout<<"mesh mem leak \n";

            return false;
        }
        if (meshNormalVBO != nullptr) {
            meshNormalVBO->deleteVBO();
            delete meshNormalVBO;
            meshNormalVBO = nullptr;

        }
        else{
           // std::cout<<"mesh mem leak \n";

            return false;
        }
        deletedData = true;
        return true;
}
void Mesh::render(const Shader& _shader)
{
    std::lock_guard<std::mutex> lock(meshMutex);
    if(loadedData && meshVAO != nullptr && meshIBO != nullptr && meshUVVBO != nullptr && meshVBO != nullptr && meshBrightnessVBO != nullptr && meshNormalVBO != nullptr) {
        _shader.use();
        meshVAO->bind();
        meshIBO->bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
        VAO::unbind();
        IBO::unbind();
    }
}
void Mesh::loadData(const Shader& _shader){

    std::lock_guard<std::mutex> lock(meshMutex);
    if(!loadedData || clearData()) {
        _shader.use();

        meshVAO = new VAO();
        meshVAO->bind();

        meshVBO = new VBO(vertices);
        meshVBO->bind();
        meshVAO->linkToVAO(0, 3, *meshVBO);
        VBO::unbind();

        meshUVVBO = new VBO(UVs);
        meshUVVBO->bind();
        meshVAO->linkToVAO(2, 2, *meshUVVBO);
        VBO::unbind();

        meshNormalVBO = new VBO(normals);
        meshNormalVBO->bind();
        meshVAO->linkToVAO(1, 3, *meshNormalVBO);
        VBO::unbind();

        meshBrightnessVBO = new VBO(brightnessFloats);
        meshBrightnessVBO->bind();
        meshVAO->linkToVAO(_shader.getAttribLocation("aBrightness"), 1, *meshBrightnessVBO);
        VBO::unbind();
        meshIBO = new IBO(indices);
        // Unbind the VAO after setting up the main shader
        VAO::unbind();
        loadedData = true;
    }

}

