//
// Created by vodkasas on 10/04/24.
//

#include "Toolbar.h"
Toolbar::Toolbar()
{
    const float toolbarWidth = 0.75f;

    const float slotWidth = toolbarWidth / 9;

    const float halfToolbarWidth = toolbarWidth / 2.0f;

    const float halfSlotWidth = slotWidth / 2.0f + 0.005f;

    // Height of the toolbar
    const float toolbarHeight = 0.15f;

    // X coordinate for the center of the toolbar
    const float toolbarCenterX = 0.0f;

    // Y coordinate for the bottom of the toolbar
    const float toolbarBottomY = -0.95f;

    // Define the vertices of the toolbar
    vertices = {
            glm::vec2(toolbarCenterX + halfToolbarWidth, toolbarBottomY),  // Bottom right
            glm::vec2(toolbarCenterX - halfToolbarWidth, toolbarBottomY),  // Bottom left
            glm::vec2(toolbarCenterX - halfToolbarWidth, toolbarBottomY + toolbarHeight),  // Top left
            glm::vec2(toolbarCenterX - halfToolbarWidth, toolbarBottomY + toolbarHeight),  // Top left
            glm::vec2(toolbarCenterX + halfToolbarWidth, toolbarBottomY + toolbarHeight), // Top right
            glm::vec2(toolbarCenterX + halfToolbarWidth, toolbarBottomY),  // Bottom right
    };
    UVCoords = {
            glm::vec2(0.7109375f, 0.0859375f), // bottom right
            glm::vec2(0.0f, 0.0859375f), // bottom left
            glm::vec2(0.0f, 0.0f), //top left
            glm::vec2(0.0f, 0.0f), //top left
            glm::vec2(0.7109375f, 0.0f), // top right
            glm::vec2(0.7109375f, 0.0859375f), // bottom right
    };
    slotVertices = {
            glm::vec2(toolbarCenterX + halfSlotWidth, toolbarBottomY - 0.005f),  // Bottom right
            glm::vec2(toolbarCenterX - halfSlotWidth, toolbarBottomY - 0.005f),  // Bottom left
            glm::vec2(toolbarCenterX - halfSlotWidth, toolbarBottomY + toolbarHeight + 0.005f),  // Top left
            glm::vec2(toolbarCenterX - halfSlotWidth, toolbarBottomY + toolbarHeight + 0.005f),  // Top left
            glm::vec2(toolbarCenterX + halfSlotWidth, toolbarBottomY + toolbarHeight + 0.005f), // Top right
            glm::vec2(toolbarCenterX + halfSlotWidth, toolbarBottomY - 0.005f),  // Bottom right
    };
    slotUVCoords = {
            glm::vec2(0.08984375f, 0.1796875f), // bottom right
            glm::vec2(0.0f, 0.1796875f), // bottom left
            glm::vec2(0.0f, 0.0859375f), //top left
            glm::vec2(0.0f, 0.0859375f), //top left
            glm::vec2(0.09375f, 0.0859375f), // top right
            glm::vec2(0.09375f, 0.1796875f), // bottom right
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

    slotShader = new Shader("../resources/shader/UIShader.vs", "../resources/shader/UIShader.fs");
    slotShader->use();

    slotVAO = new VAO();
    slotVBO = new VBO(slotVertices);
    slotVAO->Bind();
    slotVBO->Bind();
    slotVAO->LinkToVAO(slotShader->getAttribLocation("aPos"), 2, *slotVBO);
    slotVBO->Unbind();

    slotTextureVBO = new VBO(slotUVCoords);
    slotVAO->Bind();
    slotTextureVBO->Bind();
    slotVAO->LinkToVAO(slotShader->getAttribLocation("aTexCoord"), 2, *slotTextureVBO);
    slotTextureVBO->Unbind();
}
void Toolbar::renderToolbar()
{
    shader->use();
    ToolBarVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    ToolBarVAO->Unbind();

    slotShader->use();
    slotVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, slotVertices.size());
    slotVAO->Unbind();
}
