//
// Created by metinis on 26/06/24.
//

#include "Inventory.h"
#include "WorldGen/ChunkMeshGeneration.h"

Inventory::Inventory(Toolbar& _toolbar) : toolbar(_toolbar){
    // Define the vertices of the inventory
    vertices = {
            glm::vec2(inventoryCenterX + halfInventoryWidth, inventoryBottomY),  // Bottom right
            glm::vec2(inventoryCenterX - halfInventoryWidth, inventoryBottomY),  // Bottom left
            glm::vec2(inventoryCenterX - halfInventoryWidth, inventoryBottomY + inventoryHeight),  // Top left
            glm::vec2(inventoryCenterX - halfInventoryWidth, inventoryBottomY + inventoryHeight),  // Top left
            glm::vec2(inventoryCenterX + halfInventoryWidth, inventoryBottomY + inventoryHeight), // Top right
            glm::vec2(inventoryCenterX + halfInventoryWidth, inventoryBottomY),  // Bottom right
    };
    UVCoords = {
            glm::vec2(0.6875f, 0.80859375f), // bottom right
            glm::vec2(0.0f, 0.80859375f), // bottom left
            glm::vec2(0.0f, 0.0f), //top left
            glm::vec2(0.0f, 0.0f), //top left
            glm::vec2(0.6875f, 0.0f), // top right
            glm::vec2(0.6875f, 0.80859375f), // bottom right
    };

    shader = new Shader("../resources/shader/UIShader.vs", "../resources/shader/UIShader.fs");
    vao = new VAO();
    vbo = new VBO(vertices);
    shader->use();
    vao->Bind();
    vbo->Bind();
    vao->LinkToVAO(shader->getAttribLocation("aPos"), 2, *vbo);
    VBO::Unbind();

    textureVBO = new VBO(UVCoords);
    vao->Bind();
    textureVBO->Bind();
    vao->LinkToVAO(shader->getAttribLocation("aTexCoord"), 2, *textureVBO);
    VBO::Unbind();

    loadItemsRendering();

}
void Inventory::renderInventory() const
{
    shader->use();
    vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size()));
    VAO::Unbind();
}
void Inventory::loadItemsRendering() {
    deleteItemBuffers();
    loadItemsRenderingInv();
    loadItemsRenderingToolbar();
    constexpr auto model = glm::mat4(1.0f);
    constexpr auto view = glm::mat4(1.0f);
    itemShader = new Shader("../resources/shader/itemUI.vs", "../resources/shader/itemUI.fs");
    itemShader->use();

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    // Get the video mode of the primary monitor
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    // Calculate the aspect ratio
    const float aspectRatio = static_cast<float>(mode->width) / static_cast<float>(mode->height);
    const glm::mat4 proj = glm::perspective(glm::radians(65.0f), aspectRatio, 0.1f, 100.0f);
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
void Inventory::loadItemsRenderingInv(){
    for(int i = 0; i < 9; i++){
        for(int j = 1; j < 9; j++){
            addItemDataToBuffers(i, j);
        }
    }
}
void Inventory::addItemDataToBuffers(int i, int j){
    float firstSlotOffsetX = -0.3064f;
    float offsetBetweenSlotX = 0.07655f;
    float firstSlotOffsetY = 0.55827f;
    float offsetBetweenSlotY = 0.13f;
    float scaleXZ = 0.04f;
    float scaleY = 0.065f;
    auto blockCenter = glm::vec3((firstSlotOffsetX + static_cast<float>(j-1) * offsetBetweenSlotX)/scaleXZ,
        (firstSlotOffsetY - (offsetBetweenSlotY * static_cast<float>(i))) / scaleY, 0.0f);

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.065f, 0.04f));
    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(25.0f), glm::vec3(1.0f,0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f,1.0f, 0.0f));


    if((i * 8 + j) < BlockIDMap.size()){

        inventorySlots[i][j-1] = i * 8 + j;
        if(getItemAtSlot(i, j-1) != 0 && !Block::hasCustomMesh(getItemAtSlot(i, j-1))) {
            FaceData faceDataFront = Block::GetFace(CraftMine::Faces::FRONT, BlockIDMap[getItemAtSlot(i, j-1)],
                                                    blockCenter);
            FaceData faceDataRight = Block::GetFace(CraftMine::Faces::RIGHT, BlockIDMap[getItemAtSlot(i, j-1)],
                                                    blockCenter);
            FaceData faceDataTop = Block::GetFace(CraftMine::Faces::TOP, BlockIDMap[getItemAtSlot(i, j-1)],
                                                  blockCenter);
            std::vector<glm::vec3> verts;
            verts.insert(verts.end(), faceDataFront.vertices.begin(), faceDataFront.vertices.end());
            verts.insert(verts.end(), faceDataRight.vertices.begin(), faceDataRight.vertices.end());
            verts.insert(verts.end(), faceDataTop.vertices.begin(), faceDataTop.vertices.end());

            std::vector<glm::vec2> uvCoords;
            uvCoords.insert(uvCoords.end(), faceDataFront.texCoords.begin(), faceDataFront.texCoords.end());
            uvCoords.insert(uvCoords.end(), faceDataRight.texCoords.begin(), faceDataRight.texCoords.end());
            uvCoords.insert(uvCoords.end(), faceDataTop.texCoords.begin(), faceDataTop.texCoords.end());

            std::vector<float> brightness;
            for (int q = 0; q < 4; q++) {
                itemBrightness.push_back(faceDataFront.brightness);
            }
            for (int q = 0; q < 4; q++) {
                itemBrightness.push_back(faceDataRight.brightness);
            }
            for (int q = 0; q < 4; q++) {
                itemBrightness.push_back(faceDataTop.brightness);
            }

            for (glm::vec3 vert: verts) {
                glm::mat4 translationToOrigin = glm::translate(glm::mat4(1.0f), -blockCenter);
                glm::mat4 translationBack = glm::translate(glm::mat4(1.0f), blockCenter);
                auto rotatedVert = glm::vec3(
                        scale * translationBack * rotationX * rotationY * translationToOrigin * glm::vec4(vert, 1.0f));
                itemVertices.push_back(rotatedVert);
            }
            for (glm::vec2 uvCoord: uvCoords) {
                itemUVCoords.push_back(uvCoord);
            }
            ChunkMeshGeneration::AddIndices(3, indices, indexCount);
        }
        else if(Block::hasCustomMesh(getItemAtSlot(i, j-1))){
            FaceData faceDataFront = Block::GetFace(CraftMine::Faces::FRONT, BlockIDMap[getItemAtSlot(i, j-1)],
                                                    blockCenter);
            std::vector<glm::vec3> verts;
            verts.insert(verts.end(), faceDataFront.vertices.begin(), faceDataFront.vertices.end());

            std::vector<glm::vec2> uvCoords;
            uvCoords.insert(uvCoords.end(), faceDataFront.texCoords.begin(), faceDataFront.texCoords.end());

            std::vector<float> brightness;
            for (int q = 0; q < 4; q++) {
                itemBrightness.push_back(faceDataFront.brightness);
            }

            for (glm::vec3 vert: verts) {
                glm::mat4 translationToOrigin = glm::translate(glm::mat4(1.0f), -blockCenter);
                glm::mat4 translationBack = glm::translate(glm::mat4(1.0f), blockCenter);
                auto rotatedVert = glm::vec3(
                        scale * translationBack * rotationX * rotationY * translationToOrigin * glm::vec4(vert, 1.0f));
                itemVertices.push_back(rotatedVert);
            }
            for (glm::vec2 uvCoord: uvCoords) {
                itemUVCoords.push_back(uvCoord);
            }
            ChunkMeshGeneration::AddIndices(1, indices, indexCount);
        }
    }
}
void Inventory::loadItemsRenderingToolbar() {
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.065f, 0.04f));
    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    float firstSlotOffsetX = -0.3064f;

    for (int i = 0; i < 9; i++) {
        constexpr float offsetBetweenSlotX = 0.07655f;
        constexpr float scaleXZ = 0.04f;
        constexpr float scaleY = 0.065f;
        auto blockCenter = glm::vec3((firstSlotOffsetX + static_cast<float>(i) * offsetBetweenSlotX) / scaleXZ, -0.645f / scaleY,
                                          0.0f);
        std::vector<glm::vec3> verts;
        std::vector<glm::vec2> uvCoords;
        std::vector<float> brightness;

        if (BlockIDMap[toolbar.getID(i)] != 0 && !Block::hasCustomMesh(toolbar.getID(i))) {

            toolbar.getID(i);

            loadBlockData(i, verts, uvCoords, brightness, blockCenter);

            for (glm::vec3 vert: verts) {
                glm::mat4 translationToOrigin = glm::translate(glm::mat4(1.0f), -blockCenter);
                glm::mat4 translationBack = glm::translate(glm::mat4(1.0f), blockCenter);
                auto rotatedVert = glm::vec3(
                        scale * translationBack * rotationX * rotationY * translationToOrigin * glm::vec4(vert, 1.0f));
                itemVertices.push_back(rotatedVert);
            }
            for (glm::vec2 uvCoord: uvCoords) {
                itemUVCoords.push_back(uvCoord);
            }
            ChunkMeshGeneration::AddIndices(3, indices, indexCount);

        }
        else if(Block::hasCustomMesh(toolbar.getID(i))){
           toolbar.getID(i);

            loadCustomData(i, verts, uvCoords, brightness, blockCenter);

            for (glm::vec3 vert: verts) {
                glm::mat4 translationToOrigin = glm::translate(glm::mat4(1.0f), -blockCenter);
                glm::mat4 translationBack = glm::translate(glm::mat4(1.0f), blockCenter);
                auto rotatedVert = glm::vec3(
                        scale * translationBack * rotationX * rotationY * translationToOrigin * glm::vec4(vert, 1.0f));
                itemVertices.push_back(rotatedVert);
            }
            for (glm::vec2 uvCoord: uvCoords) {
                itemUVCoords.push_back(uvCoord);
            }
            ChunkMeshGeneration::AddIndices(3, indices, indexCount);
        }

    }
}
void Inventory::loadBlockData(const int i, std::vector<glm::vec3> &verts, std::vector<glm::vec2> &uvCoords, std::vector<float> &brightness, const glm::vec3 &blockCenter){

    FaceData faceDataFront = Block::GetFace(CraftMine::Faces::FRONT, BlockIDMap[toolbar.getID(i)],
                                            blockCenter);
    FaceData faceDataRight = Block::GetFace(CraftMine::Faces::RIGHT, BlockIDMap[toolbar.getID(i)],
                                            blockCenter);
    FaceData faceDataTop = Block::GetFace(CraftMine::Faces::TOP, BlockIDMap[toolbar.getID(i)],
                                          blockCenter);
    verts.insert(verts.end(), faceDataFront.vertices.begin(), faceDataFront.vertices.end());
    verts.insert(verts.end(), faceDataRight.vertices.begin(), faceDataRight.vertices.end());
    verts.insert(verts.end(), faceDataTop.vertices.begin(), faceDataTop.vertices.end());

    uvCoords.insert(uvCoords.end(), faceDataFront.texCoords.begin(), faceDataFront.texCoords.end());
    uvCoords.insert(uvCoords.end(), faceDataRight.texCoords.begin(), faceDataRight.texCoords.end());
    uvCoords.insert(uvCoords.end(), faceDataTop.texCoords.begin(), faceDataTop.texCoords.end());

    for (int q = 0; q < 4; q++) {
        itemBrightness.push_back(faceDataFront.brightness);
    }
    for (int q = 0; q < 4; q++) {
        itemBrightness.push_back(faceDataRight.brightness);
    }
    for (int q = 0; q < 4; q++) {
        itemBrightness.push_back(faceDataTop.brightness);
    }

}
void Inventory::loadCustomData(const int i, std::vector<glm::vec3> &verts, std::vector<glm::vec2> &uvCoords, std::vector<float> &brightness, const glm::vec3 &blockCenter){

    FaceData faceDataFront = Block::GetFace(CraftMine::Faces::FRONT, BlockIDMap[toolbar.getID(i)],
                                            blockCenter);
    verts.insert(verts.end(), faceDataFront.vertices.begin(), faceDataFront.vertices.end());

    uvCoords.insert(uvCoords.end(), faceDataFront.texCoords.begin(), faceDataFront.texCoords.end());

    for (int q = 0; q < 4; q++) {
        itemBrightness.push_back(faceDataFront.brightness);
    }
}

void Inventory::renderItems() const{
    itemShader->use();
    itemVAO->Bind();
    itemIBO->Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    VAO::Unbind();
    IBO::Unbind();
}

unsigned char Inventory::getItemAtSlot(const int x, const int y) const {
    return inventorySlots[x][y];
}
// Function to determine which slot is being pressed
unsigned char Inventory::determineSlotBlockID(const float mouseX, const float mouseY) const {
    // Inventory bounds
    constexpr  float minX = 0.33f;
    constexpr  float minY = 0.19f;
    constexpr  float maxX = 0.63f;
    constexpr  float maxY = 0.77f;

    // Inventory dimensions
    constexpr  int slotsX = 8;
    constexpr  int slotsY = 9;

    // Calculate slot width and height
    constexpr  float slotWidth = (maxX - minX) / slotsX;
    constexpr  float slotHeight = (maxY - minY) / slotsY;

    // Ensure the mouse click is within the inventory bounds
    if (mouseX < minX || mouseX > maxX || mouseY < minY || mouseY > maxY) {
        return 0;
    }

    // Calculate the column (xIndex) and row (yIndex) of the slot
    const int xIndex = static_cast<int>((mouseX - minX) / slotWidth);
    const int yIndex = static_cast<int>((mouseY - minY) / slotHeight);
    return getItemAtSlot(yIndex, xIndex);
}
int Inventory::determineToolbarIndex(const float mouseX, const float mouseY){

// Inventory bounds
    constexpr float minX = 0.33f;
    constexpr float minY = 0.79f;
    constexpr float maxX = 0.67f;
    constexpr float maxY = 0.85f;

    // Inventory dimensions
    constexpr int slotsX = 9;

    // Calculate slot width and height
    constexpr float slotWidth = (maxX - minX) / slotsX;

    // Ensure the mouse click is within the inventory bounds
    if (mouseX < minX || mouseX > maxX || mouseY < minY || mouseY > maxY) {
        return -1;
    }

    // Calculate the column (xIndex) and row (yIndex) of the slot
    const int xIndex = static_cast<int>((mouseX - minX) / slotWidth);

    return xIndex;
}

void Inventory::deleteItemBuffers(){
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
