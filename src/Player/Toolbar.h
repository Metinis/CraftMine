//
// Created by vodkasas on 10/04/24.
//

#ifndef CRAFTMINE_TOOLBAR_H
#define CRAFTMINE_TOOLBAR_H


#include "Graphics/Shader.h"
#include "Graphics/VBO.h"
#include "Graphics/VAO.h"
#include "Graphics/IBO.h"

class Toolbar {
private:
    const float toolbarWidth = 0.75f;

    const float slotWidth = toolbarWidth / 9.0f;

    const float halfToolbarWidth = toolbarWidth / 2.0f;

    const float halfSlotWidth = slotWidth / 2.0f;

    // Height of the toolbar
    const float toolbarHeight = 0.15f;

    // X coordinate for the center of the toolbar
    float toolbarCenterX = 0.0f;

    // Y coordinate for the bottom of the toolbar
    const float toolbarBottomY = -0.95f;

    VAO* ToolBarVAO = nullptr;
    VBO* ToolBarVBO = nullptr;
    VBO* textureVBO = nullptr;

    VAO* slotVAO = nullptr;
    VBO* slotVBO = nullptr;
    VBO* slotTextureVBO = nullptr;

    VAO* itemVAO = nullptr;
    VBO* itemVBO = nullptr;
    VBO* itemUVVBO = nullptr;
    VBO* itemBrightnessVBO = nullptr;
    IBO* itemIBO = nullptr;

    std::vector<GLuint> indices; //used for rendering blocks
    int indexCount = 0;

    Shader* shader = nullptr;
    Shader* itemShader = nullptr;

    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVCoords;

    std::vector<glm::vec2> slotVertices;
    std::vector<glm::vec2> slotUVCoords;

    std::vector<glm::vec3> itemVertices;
    std::vector<glm::vec2> itemUVCoords;

    std::vector<float> itemBrightness;


public:

    unsigned char toolbarItems[9] = {0};

    int slot;
    Toolbar();
    void renderToolbar() const;
    void changeSlot(int currentSlot);
    void changeSlotPositive();
    void changeSlotNegative();
    unsigned char getID(unsigned char _slot) const;
    void setID(unsigned char id, unsigned char _slot);
    void renderItems() const;

    void renderSlot() const;

    void deleteItemBuffers();

    void loadItemsRendering();
};



#endif //CRAFTMINE_TOOLBAR_H
