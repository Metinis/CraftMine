//
// Created by denis on 2024-02-05.
//

#ifndef CRAFTMINE_CROSSHAIR_H
#define CRAFTMINE_CROSSHAIR_H

#include "VBO.h"
#include "IBO.h"
#include "VAO.h"
#include "Shader.h"


class Crosshair {
private:
    VAO* CrosshairVAO;
    VBO* CrosshairVBO;
    VBO* UVVBO;
    Shader* shader;

    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVCoords;
public:
    Crosshair();
    void renderCrosshair();
};


#endif //CRAFTMINE_CROSSHAIR_H
