//
// Created by vodkasas on 10/04/24.
//

#include "Toolbar.h"
#include "ChunkMeshGeneration.h"

Toolbar::Toolbar()
{
    slot = 0;



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

    //slotShader = new Shader("../resources/shader/UIShader.vs", "../resources/shader/UIShader.fs");
    //slotShader->use();

    slotVAO = new VAO();
    slotVBO = new VBO(slotVertices);
    slotVAO->Bind();
    slotVBO->Bind();
    slotVAO->LinkToVAO(shader->getAttribLocation("aPos"), 2, *slotVBO);
    slotVBO->Unbind();

    slotTextureVBO = new VBO(slotUVCoords);
    slotVAO->Bind();
    slotTextureVBO->Bind();
    slotVAO->LinkToVAO(shader->getAttribLocation("aTexCoord"), 2, *slotTextureVBO);
    slotTextureVBO->Unbind();



    int j = 0;
    for(unsigned char & toolbarItem : toolbarItems){
        while(!Block::isSolid(j))
        {
            j++;
        }
        toolbarItem = j;

        j++;
    }

    loadItemsRendering();
}
void Toolbar::changeSlot(int currentSlot) {
    slot = currentSlot;

    toolbarCenterX = -halfToolbarWidth + slotWidth / 2 + currentSlot * slotWidth;

    slotVertices = {
            glm::vec2(toolbarCenterX + halfSlotWidth, toolbarBottomY - 0.005f),  // Bottom right
            glm::vec2(toolbarCenterX - halfSlotWidth, toolbarBottomY - 0.005f),  // Bottom left
            glm::vec2(toolbarCenterX - halfSlotWidth, toolbarBottomY + toolbarHeight + 0.005f),  // Top left
            glm::vec2(toolbarCenterX - halfSlotWidth, toolbarBottomY + toolbarHeight + 0.005f),  // Top left
            glm::vec2(toolbarCenterX + halfSlotWidth, toolbarBottomY + toolbarHeight + 0.005f), // Top right
            glm::vec2(toolbarCenterX + halfSlotWidth, toolbarBottomY - 0.005f),  // Bottom right
    };
    slotVAO->Bind();
    slotVBO->SetNewData(slotVertices);
    slotVBO->Bind();
    slotVAO->LinkToVAO(shader->getAttribLocation("aPos"), 2, *slotVBO);
    slotVBO->Unbind();
}
void Toolbar::changeSlotPositive() {
    if(slot == 8){
        slot = 0;
    }
    else {
        slot++;
    }
    changeSlot(slot);
}
void Toolbar::changeSlotNegative() {
    if(slot == 0){
        slot = 8;
    }
    else {
        slot--;
    }
    changeSlot(slot);
}

void Toolbar::renderToolbar()
{
    shader->use();
    ToolBarVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    ToolBarVAO->Unbind();

    //slotShader->use();
    slotVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, slotVertices.size());
    slotVAO->Unbind();


}

unsigned char Toolbar::getID(unsigned char _slot) {
    return toolbarItems[_slot];
}

void Toolbar::setID(unsigned char id, unsigned char _slot) {
    toolbarItems[_slot] = id;
}

void Toolbar::renderItems() {
    itemShader->use();
    itemVAO->Bind();
    itemIBO->Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    itemVAO->Unbind();
    itemIBO->Unbind();
}

void Toolbar::loadItemsRendering() {
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    for(int i = 0; i < 9; i++){
        toolbarCenterX = -halfToolbarWidth * 22.0f + slotWidth * 22.0f  / 2 + i * slotWidth* 22.0f ;

        FaceData faceData = Block::GetFace(CraftMine::Faces::FRONT, BlockIDMap[toolbarItems[i]],
                                           glm::vec3((toolbarCenterX),-10.9f,-20.0f));
        //view = glm::rotate(view, glm::radians(-45.0f), glm::vec3((toolbarCenterX),1.0f,-20.0f)); // Rotate around y-axis
        //model = glm::rotate(model, glm::radians(45.0f), glm::vec3((toolbarCenterX),-11.4f,-20.0f));
        for(glm::vec3 vert : faceData.vertices){
            itemVertices.push_back(vert);
        }
        for(glm::vec2 uvCoord : faceData.texCoords){
            itemUVCoords.push_back(uvCoord);
        }
        ChunkMeshGeneration::AddIndices(1, indices, indexCount);

    }
    itemShader = new Shader("../resources/shader/itemUI.vs", "../resources/shader/itemUI.fs");
    itemShader->use();




    //view = glm::rotate(view, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around z-axis

    glm::mat4 proj = glm::perspective(glm::radians(65.0f), 16.0f / 9.0f, 0.2f, 10000.0f);
    //model = glm::scale(model, glm::vec3(1.0f));
    itemShader->setMat4("model", model);
    itemShader->setMat4("view", view);
    itemShader->setMat4("projection", proj);

    itemVAO = new VAO();
    itemVBO = new VBO(itemVertices);
    itemVAO->Bind();
    itemVBO->Bind();
    itemVAO->LinkToVAO(itemShader->getAttribLocation("aPos"), 3, *itemVBO);
    itemVBO->Unbind();

    itemUVVBO = new VBO(itemUVCoords);
    itemVAO->Bind();
    itemUVVBO->Bind();
    itemVAO->LinkToVAO(itemShader->getAttribLocation("aTexCoord"), 2, *itemUVVBO);
    itemUVVBO->Unbind();

    itemIBO = new IBO(indices);
}
