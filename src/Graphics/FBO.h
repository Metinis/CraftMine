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
    unsigned int texture;
    unsigned int RBO;
    unsigned int width, height;

    void initialiseTexture();

    void initialiseDepthMap();

    void initialiseRBO();

public:
    FBO(int _width, int _height);
    //initialises the standard FBO
    void initialiseTextureFBO();

    void initialiseDepthFBO();

    void setDimension(int _width, int _height);

    void bindForRender() const;

    void bindForRead() const;

    static void Unbind();

    void Delete();
};
