//
// Created by metinis on 28/06/24.
//

#ifndef CRAFTMINE_CURSORBLOCK_H
#define CRAFTMINE_CURSORBLOCK_H


#include "Shader.h"
#include "VAO.h"
#include "IBO.h"

class CursorBlock {
private:

    Shader* itemShader = nullptr;

    VAO* itemVAO = nullptr;
    VBO* itemVBO = nullptr;
    VBO* itemUVVBO = nullptr;
    VBO* itemBrightnessVBO = nullptr;
    IBO* itemIBO = nullptr;

    std::vector<GLuint> indices; //for blocks
    int indexCount = 0;

    std::vector<glm::vec3> itemVertices;
    std::vector<glm::vec2> itemUVCoords;

    std::vector<float> itemBrightness;

    int screenWidth{};
    int screenHeight{};

    double mouseX{};
    double mouseY{};

public:
    void setScreenDimensions(const int& width, const int& height);
    void loadBlockRendering(unsigned char blockID);
    void renderBlockOnCursor() const;
    void deleteBuffers();

    unsigned char currentBlock;

    void deleteBufferData();

    void setMousePosCoordinates(const double &x, const double &y);
};


#endif //CRAFTMINE_CURSORBLOCK_H
