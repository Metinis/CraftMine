//
// Created by vodkasas on 10/04/24.
//

#ifndef CRAFTMINE_TOOLBAR_H
#define CRAFTMINE_TOOLBAR_H


#include "vec2.hpp"
#include "Shader.h"
#include "VBO.h"
#include "VAO.h"

class Toolbar {
private:
    VAO* ToolBarVAO;
    VBO* ToolBarVBO;
    VBO* textureVBO;
    Shader* shader;

    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVCoords;
public:
    Toolbar();
    void renderToolbar();
};



#endif //CRAFTMINE_TOOLBAR_H
