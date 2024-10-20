//
// Created by Denis Muraska on 12/10/2024.
//

#include "UBO.h"

#include "mat4x4.hpp"

UBO::UBO() {
    glGenBuffers(1, &matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void UBO::bind() const {
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
}
void UBO::unbind() const {
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
