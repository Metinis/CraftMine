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
            meshVAO->Delete();
            delete meshVAO;
            meshVAO = nullptr;
        }
        else{
           // std::cout<<"mesh mem leak \n";
           return false;
        }

        if (meshVBO != nullptr) {
            meshVBO->Delete();
            delete meshVBO;
            meshVBO = nullptr;
        }
        else{
           // std::cout<<"mesh mem leak \n";

            return false;
        }
        if (meshUVVBO != nullptr) {
            meshUVVBO->Delete();
            delete meshUVVBO;
            meshUVVBO = nullptr;
        }
        else{
          //  std::cout<<"mesh mem leak \n";

            return false;
        }
        if (meshIBO != nullptr) {
            meshIBO->Delete();
            delete meshIBO;
            meshIBO = nullptr;
        }
        else{
          //  std::cout<<"mesh mem leak \n";

            return false;
        }
        if (meshBrightnessVBO != nullptr) {
            meshBrightnessVBO->Delete();
            delete meshBrightnessVBO;
            meshBrightnessVBO = nullptr;
        }
        else{
          //  std::cout<<"mesh mem leak \n";

            return false;
        }
        if (meshNormalVBO != nullptr) {
            meshNormalVBO->Delete();
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
        meshVAO->Bind();
        meshIBO->Bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
        VAO::Unbind();
        IBO::Unbind();
    }
}
void Mesh::loadData(const Shader& _shader){

    std::lock_guard<std::mutex> lock(meshMutex);
    if(!loadedData || clearData()) {
        _shader.use();

        meshVAO = new VAO();
        meshVAO->Bind();

        meshVBO = new VBO(vertices);
        meshVBO->Bind();
        meshVAO->LinkToVAO(0, 3, *meshVBO);
        VBO::Unbind();

        meshUVVBO = new VBO(UVs);
        meshUVVBO->Bind();
        meshVAO->LinkToVAO(2, 2, *meshUVVBO);
        VBO::Unbind();

        meshNormalVBO = new VBO(normals);
        meshNormalVBO->Bind();
        meshVAO->LinkToVAO(1, 3, *meshNormalVBO);
        VBO::Unbind();

        meshBrightnessVBO = new VBO(brightnessFloats);
        meshBrightnessVBO->Bind();
        meshVAO->LinkToVAO(_shader.getAttribLocation("aBrightness"), 1, *meshBrightnessVBO);
        VBO::Unbind();
        meshIBO = new IBO(indices);
        // Unbind the VAO after setting up the main shader
        VAO::Unbind();
        loadedData = true;
    }

}

