//
// Created by vodkasas on 10/04/24.
//

#include "Toolbar.h"
#include "WorldGen/ChunkMeshGeneration.h"

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
            glm::vec2(0.09375f, 0.1796875f), // bottom right
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
    VBO::Unbind();

    textureVBO = new VBO(UVCoords);
    ToolBarVAO->Bind();
    textureVBO->Bind();
    ToolBarVAO->LinkToVAO(shader->getAttribLocation("aTexCoord"), 2, *textureVBO);
    VBO::Unbind();

    slotVAO = new VAO();
    slotVBO = new VBO(slotVertices);
    slotVAO->Bind();
    slotVBO->Bind();
    slotVAO->LinkToVAO(shader->getAttribLocation("aPos"), 2, *slotVBO);
    VBO::Unbind();

    slotTextureVBO = new VBO(slotUVCoords);
    slotVAO->Bind();
    slotTextureVBO->Bind();
    slotVAO->LinkToVAO(shader->getAttribLocation("aTexCoord"), 2, *slotTextureVBO);
    VBO::Unbind();



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
void Toolbar::changeSlot(const int currentSlot) {
    slot = currentSlot;
    const float adjustedToolbarWidth = toolbarWidth * 180/182;
    const float adjustedHalfToolbarWidth = adjustedToolbarWidth / 2.0f;
    const float adjustedSlotWidth = adjustedToolbarWidth / 9.0f;
    const float adjustedHalfSlotWidth = adjustedSlotWidth / 2.0f;

    toolbarCenterX = -adjustedHalfToolbarWidth + adjustedHalfSlotWidth + adjustedSlotWidth * static_cast<float>(currentSlot);
    constexpr float offset = 0.005f;

    slotVertices = {
            glm::vec2(toolbarCenterX + halfSlotWidth + offset, toolbarBottomY - offset),  // Bottom right
            glm::vec2(toolbarCenterX - halfSlotWidth - offset, toolbarBottomY - offset),  // Bottom left
            glm::vec2(toolbarCenterX - halfSlotWidth - offset, toolbarBottomY + toolbarHeight + offset),  // Top left
            glm::vec2(toolbarCenterX - halfSlotWidth - offset, toolbarBottomY + toolbarHeight + offset),  // Top left
            glm::vec2(toolbarCenterX + halfSlotWidth + offset, toolbarBottomY + toolbarHeight + offset), // Top right
            glm::vec2(toolbarCenterX + halfSlotWidth + offset, toolbarBottomY - offset),  // Bottom right
    };
    slotVAO->Bind();
    slotVBO->SetNewData(slotVertices);
    slotVBO->Bind();
    slotVAO->LinkToVAO(shader->getAttribLocation("aPos"), 2, *slotVBO);
    VBO::Unbind();
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

void Toolbar::renderToolbar() const
{
    shader->use();
    ToolBarVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size()));
    VAO::Unbind();
}
void Toolbar::renderSlot() const{
    shader->use();
    slotVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(slotVertices.size()));
    VAO::Unbind();
}

unsigned char Toolbar::getID(const unsigned char _slot) const{
    return toolbarItems[_slot];
}

void Toolbar::setID(const unsigned char id, const unsigned char _slot) {
    toolbarItems[_slot] = id;
}

void Toolbar::renderItems() const{
    itemShader->use();
    itemVAO->Bind();
    itemIBO->Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    VAO::Unbind();
    IBO::Unbind();
}

void Toolbar::loadItemsRendering() {
    deleteItemBuffers();
    constexpr auto model = glm::mat4(1.0f);
    constexpr auto view = glm::mat4(1.0f);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.0375f, 0.065f, 0.0375f));
    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(25.0f), glm::vec3(1.0f,0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f,1.0f, 0.0f));
    for(int i = 0; i < 9; i++){
        if(toolbarItems[i] != 0){
            toolbarCenterX = -halfToolbarWidth * 22.0f + (slotWidth) * 22.0f  / 2 + static_cast<float>(i) * 1.2f * (slotWidth) * 22.0f ;

            auto blockCenter = glm::vec3(toolbarCenterX - 1.47f, -13.5f, 0.0f);

            std::vector<glm::vec3> verts;

            std::vector<glm::vec2> uvCoords;

            std::vector<float> brightness;

            if(!Block::hasCustomMesh(toolbarItems[i])){
                FaceData faceDataFront = Block::GetFace(CraftMine::Faces::FRONT, BlockIDMap[toolbarItems[i]],
                                                        blockCenter);
                FaceData faceDataRight = Block::GetFace(CraftMine::Faces::RIGHT, BlockIDMap[toolbarItems[i]],
                                                        blockCenter);
                FaceData faceDataTop = Block::GetFace(CraftMine::Faces::TOP, BlockIDMap[toolbarItems[i]],
                                                      blockCenter);
                verts.insert(verts.end(), faceDataFront.vertices.begin(), faceDataFront.vertices.end());
                verts.insert(verts.end(), faceDataRight.vertices.begin(), faceDataRight.vertices.end());
                verts.insert(verts.end(), faceDataTop.vertices.begin(), faceDataTop.vertices.end());

                uvCoords.insert(uvCoords.end(), faceDataFront.texCoords.begin(), faceDataFront.texCoords.end());
                uvCoords.insert(uvCoords.end(), faceDataRight.texCoords.begin(), faceDataRight.texCoords.end());
                uvCoords.insert(uvCoords.end(), faceDataTop.texCoords.begin(), faceDataTop.texCoords.end());

                for(int q = 0; q < 4; q++){
                    itemBrightness.push_back(faceDataFront.brightness);
                }
                for(int q = 0; q < 4; q++){
                    itemBrightness.push_back(faceDataRight.brightness);
                }
                for(int q = 0; q < 4; q++){
                    itemBrightness.push_back(faceDataTop.brightness);
                }

                ChunkMeshGeneration::AddIndices(3, indices, indexCount);
            }
            else{
                FaceData faceDataFront = Block::GetFace(CraftMine::Faces::FRONT, BlockIDMap[toolbarItems[i]],
                                                        blockCenter);
                verts.insert(verts.end(), faceDataFront.vertices.begin(), faceDataFront.vertices.end());

                uvCoords.insert(uvCoords.end(), faceDataFront.texCoords.begin(), faceDataFront.texCoords.end());

                for(int q = 0; q < 4; q++){
                    itemBrightness.push_back(faceDataFront.brightness);
                }

                ChunkMeshGeneration::AddIndices(1, indices, indexCount);
            }


            for(glm::vec3 vert : verts){
                glm::mat4 translationToOrigin = glm::translate(glm::mat4(1.0f), -blockCenter);
                glm::mat4 translationBack = glm::translate(glm::mat4(1.0f), blockCenter);
                auto rotatedVert = glm::vec3(scale * translationBack * rotationX * rotationY * translationToOrigin * glm::vec4(vert, 1.0f));
                itemVertices.push_back(rotatedVert);
            }
            for(glm::vec2 uvCoord : uvCoords){
                itemUVCoords.push_back(uvCoord);
            }
        }


    }
    itemShader = new Shader("../resources/shader/itemUI.vs", "../resources/shader/itemUI.fs");
    itemShader->use();

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    // Get the video mode of the primary monitor
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    // Calculate the aspect ratio
    float aspectRatio = static_cast<float>(mode->width) / static_cast<float>(mode->height);
    glm::mat4 proj = glm::perspective(glm::radians(65.0f), aspectRatio, 0.1f, 100.0f);

    itemShader->setMat4("model", model);
    itemShader->setMat4("view", view);
    itemShader->setMat4("projection", proj);

    itemVAO = new VAO();
    itemVBO = new VBO(itemVertices);
    itemVAO->Bind();
    itemVBO->Bind();
    itemVAO->LinkToVAO(itemShader->getAttribLocation("aPos"), 3, *itemVBO);
    VBO::Unbind();

    itemUVVBO = new VBO(itemUVCoords);
    itemVAO->Bind();
    itemUVVBO->Bind();
    itemVAO->LinkToVAO(itemShader->getAttribLocation("aTexCoord"), 2, *itemUVVBO);
    VBO::Unbind();

    itemBrightnessVBO = new VBO(itemBrightness);
    itemVAO->Bind();
    itemBrightnessVBO->Bind();
    itemVAO->LinkToVAO(itemShader->getAttribLocation("aBrightness"), 1, *itemBrightnessVBO);
    VBO::Unbind();

    itemIBO = new IBO(indices);
}
void Toolbar::deleteItemBuffers(){
    if(itemShader != nullptr){
        delete itemShader;
        itemShader = nullptr;
    }
    if(itemVAO != nullptr){
        delete itemVAO;
        itemVAO = nullptr;
    }if(itemVBO != nullptr){
        delete itemVBO;
        itemVBO = nullptr;
    }if(itemBrightnessVBO != nullptr){
        delete itemBrightnessVBO;
        itemBrightnessVBO = nullptr;
    }if(itemUVVBO != nullptr){
        delete itemUVVBO;
        itemUVVBO = nullptr;
    }if(itemIBO != nullptr){
        delete itemIBO;
        itemIBO = nullptr;
    }
    indices.clear();
    indexCount = 0;
    itemVertices.clear();
    itemUVCoords.clear();
    itemBrightness.clear();
}
