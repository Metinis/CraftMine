//
// Created by denis on 2024-02-05.
//

#ifndef CRAFTMINE_UI_H
#define CRAFTMINE_UI_H

#include "VBO.h"
#include "IBO.h"
#include "VAO.h"
#include "Shader.h"


class UI {
private:
    VAO* uiVAO;
    VBO* uiVBO;
    Shader* shader;

    std::vector<glm::vec2> vertices;
public:
    UI();
    void renderCrosshair();
};


#endif //CRAFTMINE_UI_H
