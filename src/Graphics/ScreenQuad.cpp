//
// Created by denis on 2024-03-13.
//

#include "ScreenQuad.h"

ScreenQuad::ScreenQuad(){
    const std::vector<float> rectangleVertices =
            {
                    // Coords               // texCoords
                    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                    -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,

                    1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
                    1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
                    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f
            };
    quadVAO = new VAO();
    quadVBO = new VBO(rectangleVertices);
    quadVAO->bind();
    quadVBO->bind();
    quadVAO->linkToVAO(0, 3, 5 * sizeof(float), ((void*)0), *quadVBO);
    quadVAO->linkToVAO(1, 2, 5 * sizeof(float), (void*)(3 * sizeof(float)), *quadVBO);
    quadVAO->unbind();
    quadVBO->unbind();
}
void ScreenQuad::renderQuad(Shader& shader) const {
    shader.use();
    quadVAO->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
ScreenQuad::~ScreenQuad(){
    quadVAO->deleteVAO();
    quadVBO->deleteVBO();
    delete quadVAO;
    delete quadVBO;
}
