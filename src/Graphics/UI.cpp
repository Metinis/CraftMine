//
// Created by denis on 2024-02-05.
//

#include "UI.h"

UI::UI()
{
    vertices = {
            glm::vec2(-0.01f, 0.0f),
            glm::vec2(0.01f, 0.0f),
            glm::vec2(0.0f, -0.02f),
            glm::vec2(0.0f, 0.02f)
    };
    shader = new Shader("../resources/shader/UIShader.vs", "../resources/shader/UIShader.fs");
    uiVAO = new VAO();
    uiVBO = new VBO(vertices);
    shader->use();
    uiVAO->Bind();
    uiVBO->Bind();
    uiVAO->LinkToVAO(shader->getAttribLocation("aPos"), 2, *uiVBO);
    uiVBO->Unbind();
}
void UI::renderCrosshair()
{
    shader->use();
    uiVAO->Bind();
    glDrawArrays(GL_LINES, 0, vertices.size());
    uiVAO->Unbind();
}
