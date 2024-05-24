#pragma once
#include "VBO.h"
#include "VAO.h"
#include "IBO.h"
#include "FBO.h"
#include "Texture.h"
#include <glm/vec3.hpp>
#include <utility>
#include "Shader.h"

class Mesh {
private:
    VAO *meshVAO = nullptr;
    VBO *meshVBO = nullptr;
    VBO *meshUVVBO = nullptr;
    VBO *meshBrightnessVBO = nullptr;
    VBO *meshNormalVBO = nullptr;
    IBO *meshIBO = nullptr;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> UVs;
    std::vector<GLuint> indices;
    std::vector<float> brightnessFloats;


public:
    bool loadedData = false;
    void setData(std::vector<glm::vec3> _vertices, std::vector<glm::vec3> _normals, std::vector<glm::vec2> _UVs, std::vector<GLuint> _indices, std::vector<float> _brightnessFloats);
    void clearData();
    void render(Shader& _shader);
    //void renderShadow(Shader& _shader);
    void loadData(Shader& _shader);

    ~Mesh()
    {
        loadedData = false;
        vertices.clear();
        UVs.clear();
        indices.clear();
        brightnessFloats.clear();
        normals.clear();
        clearData();
    }
};

