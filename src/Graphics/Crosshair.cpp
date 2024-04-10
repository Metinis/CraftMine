//
// Created by denis on 2024-02-05.
//

#include "Crosshair.h"

Crosshair::Crosshair()
{
    vertices = {
            glm::vec2(-0.01f, 0.0f),
            glm::vec2(0.01f, 0.0f),
            glm::vec2(0.0f, -0.02f),
            glm::vec2(0.0f, 0.02f)
    };
    shader = new Shader("../resources/shader/crosshair.vs", "../resources/shader/crosshair.fs");
    CrosshairVAO = new VAO();
    CrosshairVBO = new VBO(vertices);
    shader->use();
    CrosshairVAO->Bind();
    CrosshairVBO->Bind();
    CrosshairVAO->LinkToVAO(shader->getAttribLocation("aPos"), 2, *CrosshairVBO);
    CrosshairVBO->Unbind();
}
void Crosshair::renderCrosshair()
{
    shader->use();
    CrosshairVAO->Bind();
    glDrawArrays(GL_LINES, 0, vertices.size());
    CrosshairVAO->Unbind();
}
