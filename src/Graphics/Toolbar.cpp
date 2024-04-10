//
// Created by vodkasas on 10/04/24.
//

#include "Toolbar.h"
Toolbar::Toolbar()
{
    // Width of each toolbar slot
    const float slotWidth = 0.75f;

    // Calculate half of the slot width for convenience
    const float halfSlotWidth = slotWidth / 2.0f;

    // Height of the toolbar
    const float toolbarHeight = 0.15f;

    // X coordinate for the center of the toolbar
    const float toolbarCenterX = 0.0f;

    // Y coordinate for the bottom of the toolbar
    const float toolbarBottomY = -0.95f;

    // Define the vertices of the toolbar
    vertices = {
            glm::vec2(toolbarCenterX + halfSlotWidth, toolbarBottomY),  // Bottom right
            glm::vec2(toolbarCenterX - halfSlotWidth, toolbarBottomY),  // Bottom left
            glm::vec2(toolbarCenterX - halfSlotWidth, toolbarBottomY + toolbarHeight),  // Top left
            glm::vec2(toolbarCenterX - halfSlotWidth, toolbarBottomY + toolbarHeight),  // Top left
            glm::vec2(toolbarCenterX + halfSlotWidth, toolbarBottomY + toolbarHeight), // Top right
            glm::vec2(toolbarCenterX + halfSlotWidth, toolbarBottomY),  // Bottom right
    };
    UVCoords = {
            glm::vec2(0.7109375f, 0.0859375f), // bottom right
            glm::vec2(0.0f, 0.0859375f), // bottom left
            glm::vec2(0.0f, 0.0f), //top left
            glm::vec2(0.0f, 0.0f), //top left
            glm::vec2(0.7109375f, 0.0f), // top right
            glm::vec2(0.7109375f, 0.0859375f), // bottom right
    };
    shader = new Shader("../resources/shader/UIShader.vs", "../resources/shader/UIShader.fs");
    ToolBarVAO = new VAO();
    ToolBarVBO = new VBO(vertices);
    shader->use();
    ToolBarVAO->Bind();
    ToolBarVBO->Bind();
    ToolBarVAO->LinkToVAO(shader->getAttribLocation("aPos"), 2, *ToolBarVBO);
    ToolBarVBO->Unbind();

    textureVBO = new VBO(UVCoords);
    ToolBarVAO->Bind();
    textureVBO->Bind();
    ToolBarVAO->LinkToVAO(shader->getAttribLocation("aTexCoord"), 2, *textureVBO);
    textureVBO->Unbind();
}
void Toolbar::renderToolbar()
{
    shader->use();
    ToolBarVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    ToolBarVAO->Unbind();
}
