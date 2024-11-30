#pragma once
#include "VBO.h"
#include "VAO.h"
#include "IBO.h"
#include "FBO.h"
#include "Texture.h"
#include <glm/vec3.hpp>
#include <utility>
#include "Shader.h"
#include <iostream>
#include <mutex>

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
    std::vector<glm::vec4> rgbiValues;

    std::mutex meshMutex;

    bool clearData();

public:
    bool loadedData = false;
    bool deletedData = false;
    bool beingRendered = false;
    void setData(std::vector<glm::vec3> _vertices, std::vector<glm::vec3> _normals, std::vector<glm::vec2> UVs_, std::vector<GLuint> _indices, std::vector<glm::vec4> _rgbiValues);
    void render(const Shader& _shader);
    //void renderShadow(Shader& _shader);
    void loadData(const Shader& _shader);

    ~Mesh()
    {

        std::lock_guard<std::mutex> lock(meshMutex);
        if(!clearData()){
            std::cout<<"mem leak mesh \n";
        }
    }
};

