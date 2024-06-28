//
// Created by denis on 2024-02-05.
//

#include "Crosshair.h"

Crosshair::Crosshair()
{
    /*vertices = {
            glm::vec2(-0.01f, 0.0f),
            glm::vec2(0.01f, 0.0f),
            glm::vec2(0.0f, -0.02f),
            glm::vec2(0.0f, 0.02f)
    };*/
    float centerX = 0.0;
    float halfWidth = 0.015f;
    float bottomY = -0.02f;
    float height = 0.04f;
    vertices = {
            glm::vec2(centerX + halfWidth, bottomY),  // Bottom right
            glm::vec2(centerX - halfWidth, bottomY),  // Bottom left
            glm::vec2(centerX - halfWidth, bottomY + height),  // Top left
            glm::vec2(centerX - halfWidth, bottomY + height),  // Top left
            glm::vec2(centerX + halfWidth, bottomY + height), // Top right
            glm::vec2(centerX + halfWidth, bottomY),  // Bottom right
    };
    UVCoords = {
            glm::vec2(0.98828125f, 0.046875f), // bottom right
            glm::vec2(0.9453125f, 0.046875f), // bottom left
            glm::vec2(0.9453125f,  0.01171875f ), //top left
            glm::vec2(0.9453125f,  0.01171875f ), //top left
            glm::vec2(0.98828125f,  0.01171875f ), // top right
            glm::vec2(0.98828125f, 0.046875f), // bottom right
            /*glm::vec2(0.09375f, 0.1796875f), // bottom right
            glm::vec2(0.0f, 0.1796875f), // bottom left
            glm::vec2(0.0f, 0.0859375f), //top left
            glm::vec2(0.0f, 0.0859375f), //top left
            glm::vec2(0.09375f, 0.0859375f), // top right
            glm::vec2(0.09375f, 0.1796875f), // bottom right*/
    };
    /*shader = new Shader("../resources/shader/crosshair.vs", "../resources/shader/crosshair.fs");
    CrosshairVAO = new VAO();
    CrosshairVBO = new VBO(vertices);
    shader->use();
    CrosshairVAO->Bind();
    CrosshairVBO->Bind();
    CrosshairVAO->LinkToVAO(shader->getAttribLocation("aPos"), 2, *CrosshairVBO);
    CrosshairVBO->Unbind();*/
    shader = new Shader("../resources/shader/crosshair.vs", "../resources/shader/crosshair.fs");
    CrosshairVAO = new VAO();
    CrosshairVBO = new VBO(vertices);
    shader->use();
    CrosshairVAO->Bind();
    CrosshairVBO->Bind();
    CrosshairVAO->LinkToVAO(shader->getAttribLocation("aPos"), 2, *CrosshairVBO);
    CrosshairVBO->Unbind();

    UVVBO = new VBO(UVCoords);
    CrosshairVAO->Bind();
    UVVBO->Bind();
    CrosshairVAO->LinkToVAO(shader->getAttribLocation("aTexCoord"), 2, *UVVBO);
    UVVBO->Unbind();
}
void Crosshair::renderCrosshair()
{
    shader->use();
    CrosshairVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    CrosshairVAO->Unbind();
}
