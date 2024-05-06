//
// Created by vodkasas on 10/04/24.
//

#ifndef CRAFTMINE_TOOLBAR_H
#define CRAFTMINE_TOOLBAR_H


#include "vec2.hpp"
#include "Shader.h"
#include "VBO.h"
#include "VAO.h"
#include "Block.h"
#include "BlockData.h"
#include "IBO.h"

class Toolbar {
private:
    const float toolbarWidth = 0.75f;

    const float slotWidth = toolbarWidth / 9;

    const float halfToolbarWidth = toolbarWidth / 2.0f;

    const float halfSlotWidth = slotWidth / 2.0f + 0.005f;

    // Height of the toolbar
    const float toolbarHeight = 0.15f;

    // X coordinate for the center of the toolbar
    float toolbarCenterX = 0.0f;

    // Y coordinate for the bottom of the toolbar
    const float toolbarBottomY = -0.95f;

    VAO* ToolBarVAO;
    VBO* ToolBarVBO;
    VBO* textureVBO;

    VAO* slotVAO;
    VBO* slotVBO;
    VBO* slotTextureVBO;

    VAO* itemVAO;
    VBO* itemVBO;
    VBO* itemUVVBO;
    VBO* itemBrightnessVBO;
    IBO* itemIBO;

    std::vector<GLuint> indices;
    int indexCount = 0;

    Shader* shader;
    Shader* itemShader;

    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVCoords;

    std::vector<glm::vec2> slotVertices;
    std::vector<glm::vec2> slotUVCoords;

    std::vector<glm::vec3> itemVertices;
    std::vector<glm::vec2> itemUVCoords;

    std::vector<float> itemBrightness;

    unsigned char toolbarItems[9] = {0};

    void loadItemsRendering();

public:
    int slot;
    Toolbar();
    void renderToolbar();
    void changeSlot(int currentSlot);
    void changeSlotPositive();
    void changeSlotNegative();
    unsigned char getID(unsigned char _slot);
    void setID(unsigned char id, unsigned char _slot);
    void renderItems();

    void renderSlot();
};



#endif //CRAFTMINE_TOOLBAR_H
