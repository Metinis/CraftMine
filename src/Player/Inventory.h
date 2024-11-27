//
// Created by metinis on 26/06/24.
//

#ifndef CRAFTMINE_INVENTORY_H
#define CRAFTMINE_INVENTORY_H


#include "VBO.h"
#include "VAO.h"
#include "IBO.h"
#include "Shader.h"
#include "Toolbar.h"

class Inventory {
private:
    const float inventoryWidth = 0.75f;

    const float halfInventoryWidth = inventoryWidth / 2.0f;

    // Height of the toolbar
    const float inventoryHeight = 1.5f;

    // X coordinate for the center of the toolbar
    float inventoryCenterX = 0.0f;

    // Y coordinate for the bottom of the toolbar
    const float inventoryBottomY = -0.75f;


    const float slotWidth = 0.15f * inventoryWidth;

    const float halfSlotWidth = slotWidth / 2.0f;

    VBO* vbo = nullptr;
    VBO* textureVBO = nullptr;
    VAO* vao = nullptr;
    VAO* itemVAO = nullptr;
    VBO* itemVBO = nullptr;
    VBO* itemUVVBO = nullptr;
    VBO* itemBrightnessVBO = nullptr;
    IBO* itemIBO = nullptr;

    std::vector<GLuint> indices; //for blocks
    int indexCount = 0;

    Shader* shader = nullptr;
    Shader* itemShader = nullptr;

    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVCoords;

    std::vector<glm::vec3> itemVertices;
    std::vector<glm::vec2> itemUVCoords;

    std::vector<float> itemBrightness;

    Toolbar& toolbar;

    unsigned char inventorySlots[9][8] = {0};
public:
    explicit Inventory(Toolbar& _toolbar);

    unsigned char getItemAtSlot(int x, int y) const;

    void renderInventory() const;

    void loadItemsRendering();

    void renderItems() const;

    unsigned char determineSlotBlockID(float mouseX, float mouseY) const;

    static int determineToolbarIndex(float mouseX, float mouseY);

    void loadItemsRenderingInv();

    void loadItemsRenderingToolbar();

    void deleteItemBuffers();

    void addItemDataToBuffers(int i, int j);

    void
    loadBlockData(int i, std::vector<glm::vec3> &verts, std::vector<glm::vec2> &uvCoords,
                  std::vector<float> &brightness,
                  const glm::vec3 &blockCenter);

    void
    loadCustomData(int i, std::vector<glm::vec3> &verts, std::vector<glm::vec2> &uvCoords,
                   std::vector<float> &brightness,
                   const glm::vec3 &blockCenter);
};


#endif //CRAFTMINE_INVENTORY_H
