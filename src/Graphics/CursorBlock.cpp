//
// Created by metinis on 28/06/24.
//

#include "CursorBlock.h"
#include "WorldGen/ChunkMeshGeneration.h"

void CursorBlock::loadBlockRendering(unsigned char blockID) {
    deleteBuffers();
    deleteBufferData();
    currentBlock = blockID;
    if (currentBlock != 0) {
        auto model = glm::mat4(1.0f);
        auto view = glm::mat4(1.0f);
        auto blockCenter = glm::vec3(0, 0, 0);

        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.065f, 0.04f));
        glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        std::vector<glm::vec3> verts;

        std::vector<glm::vec2> uvCoords;

        std::vector<float> brightness;
        if (!Block::hasCustomMesh(currentBlock)) {
            FaceData faceDataFront = Block::GetFace(CraftMine::Faces::FRONT, BlockIDMap[blockID],
                                                    blockCenter);
            FaceData faceDataRight = Block::GetFace(CraftMine::Faces::RIGHT, BlockIDMap[blockID],
                                                    blockCenter);
            FaceData faceDataTop = Block::GetFace(CraftMine::Faces::TOP, BlockIDMap[blockID],
                                                  blockCenter);

            verts.insert(verts.end(), faceDataFront.vertices.begin(), faceDataFront.vertices.end());
            verts.insert(verts.end(), faceDataRight.vertices.begin(), faceDataRight.vertices.end());
            verts.insert(verts.end(), faceDataTop.vertices.begin(), faceDataTop.vertices.end());

            uvCoords.insert(uvCoords.end(), faceDataFront.texCoords.begin(), faceDataFront.texCoords.end());
            uvCoords.insert(uvCoords.end(), faceDataRight.texCoords.begin(), faceDataRight.texCoords.end());
            uvCoords.insert(uvCoords.end(), faceDataTop.texCoords.begin(), faceDataTop.texCoords.end());

            for (int i = 0; i < 4; i++) {
                itemBrightness.push_back(faceDataFront.brightness);
            }
            for (int i = 0; i < 4; i++) {
                itemBrightness.push_back(faceDataRight.brightness);
            }
            for (int i = 0; i < 4; i++) {
                itemBrightness.push_back(faceDataTop.brightness);
            }

            ChunkMeshGeneration::AddIndices(3, indices, indexCount);
        } else {
            FaceData faceDataFront = Block::GetFace(CraftMine::Faces::FRONT, BlockIDMap[blockID],
                                                    blockCenter);

            verts.insert(verts.end(), faceDataFront.vertices.begin(), faceDataFront.vertices.end());

            uvCoords.insert(uvCoords.end(), faceDataFront.texCoords.begin(), faceDataFront.texCoords.end());

            for (int i = 0; i < 4; i++) {
                itemBrightness.push_back(faceDataFront.brightness);
            }

            ChunkMeshGeneration::AddIndices(1, indices, indexCount);
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

        itemShader = new Shader("../resources/shader/itemUI.vs", "../resources/shader/itemUI.fs");
        itemShader->use();
        // Get the primary monitor
        GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();

        // Get the video mode of the primary monitor
        const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);

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
        itemVBO->Unbind();

        itemUVVBO = new VBO(itemUVCoords);
        itemVAO->Bind();
        itemUVVBO->Bind();
        itemVAO->LinkToVAO(itemShader->getAttribLocation("aTexCoord"), 2, *itemUVVBO);
        itemUVVBO->Unbind();

        itemBrightnessVBO = new VBO(itemBrightness);
        itemVAO->Bind();
        itemBrightnessVBO->Bind();
        itemVAO->LinkToVAO(itemShader->getAttribLocation("aBrightness"), 1, *itemBrightnessVBO);
        itemBrightnessVBO->Unbind();

        itemIBO = new IBO(indices);
    }
}

void CursorBlock::renderBlockOnCursor() const{
    // Get mouse position
    const float xNDC = 2.0f * static_cast<float>(mouseX) / static_cast<float>(screenWidth) - 1.0f;
    const float yNDC = 1.0f - 2.0f * static_cast<float>(mouseY) / static_cast<float>(screenHeight);

    // Calculate the model matrix
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xNDC, yNDC, 0.0f));

    itemShader->use();

    itemShader->setMat4("model", model);
    itemVAO->Bind();
    itemIBO->Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    itemVAO->Unbind();
    IBO::Unbind();
}

void CursorBlock::deleteBuffers() {
    if (itemShader != nullptr) {
        delete itemShader;
        itemShader = nullptr;
    }
    if (itemVAO != nullptr) {
        delete itemVAO;
        itemVAO = nullptr;
    }
    if (itemVBO != nullptr) {
        delete itemVBO;
        itemVBO = nullptr;
    }
    if (itemBrightnessVBO != nullptr) {
        delete itemBrightnessVBO;
        itemBrightnessVBO = nullptr;
    }
    if (itemUVVBO != nullptr) {
        delete itemUVVBO;
        itemUVVBO = nullptr;
    }
    if (itemIBO != nullptr) {
        delete itemIBO;
        itemIBO = nullptr;
    }
}

void CursorBlock::deleteBufferData() {
    indices.clear();
    indexCount = 0;
    itemVertices.clear();
    itemUVCoords.clear();
    itemBrightness.clear();
}

void CursorBlock::setScreenDimensions(const int &width, const int &height) {
    screenWidth = width;
    screenHeight = height;
}

void CursorBlock::setMousePosCoordinates(const double &x, const double &y) {
    mouseX = x;
    mouseY = y;
}
