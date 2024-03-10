#pragma once
#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <glm/vec2.hpp> // glm::vec2
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <vector>
#include <glm/glm.hpp>


class FBO {
private:
    unsigned int ID;
    unsigned int textureFBO;
    unsigned int RBO;
    unsigned int width, height;

    void initialiseTexture() const;

    void initialiseRBO() const;

public:
    FBO(int _width, int _height);

    void setDimension(int _width, int _height);

    void bindForRender() const;

    void bindForRead() const;

    static void Unbind();

    void Delete();
};
