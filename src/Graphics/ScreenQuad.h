#pragma once
#include "VBO.h"
#include "VAO.h"
#include "Shader.h"


class ScreenQuad {
private:
    VBO* quadVBO;
    VAO* quadVAO;
public:
    ScreenQuad();

    void renderQuad(Shader& shader) const;

    ~ScreenQuad();

};

