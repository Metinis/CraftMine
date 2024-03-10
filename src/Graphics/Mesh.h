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
    Shader& shader;
    Shader& shadowMap;

    VAO *meshVAO = nullptr;
    VAO* shadowMapVAO = nullptr;
    VBO *meshVBO = nullptr;
    VBO *meshUVVBO = nullptr;
    VBO *meshBrightnessVBO = nullptr;
    IBO *meshIBO = nullptr;
    FBO *meshFBO = nullptr;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> UVs;
    std::vector<GLuint> indices;
    std::vector<float> brightnessFloats;
public:
    void setData(std::vector<glm::vec3> _vertices, std::vector<glm::vec2> _UVs, std::vector<GLuint> _indices, std::vector<float> _brightnessFloats);
    void clearData();
    void render();
    void loadData();

    Mesh(Shader& _shader, Shader& _shadowMap);

    ~Mesh()
    {
        vertices.clear();
        UVs.clear();
        indices.clear();
        brightnessFloats.clear();
        clearData();
    }
};

