#pragma once
#include "VBO.h"
#include "VAO.h"
#include "IBO.h"
#include "Texture.h"
#include <glm/vec3.hpp>
#include <utility>
#include "Shader.h"

class Mesh {
private:
    Shader& shader;

    VAO *meshVAO = nullptr;
    VBO *meshVBO = nullptr;
    VBO *meshUVVBO = nullptr;
    IBO *meshIBO = nullptr;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> UVs;
    std::vector<GLuint> indices;
public:
    void setData(std::vector<glm::vec3> _vertices, std::vector<glm::vec2> _UVs, std::vector<GLuint> _indices);
    void clearData();
    void render();
    void loadData();

    Mesh(Shader& _shader);

    ~Mesh()
    {
        vertices.clear();
        UVs.clear();
        indices.clear();
        clearData();
    }
};

