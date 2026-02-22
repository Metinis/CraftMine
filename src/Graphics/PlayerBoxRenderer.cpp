#include "PlayerBoxRenderer.h"
#include "Shader.h"
#include "Player/RemotePlayer.h"
#include <glm/gtc/matrix_transform.hpp>

PlayerBoxRenderer::PlayerBoxRenderer() {
    shader = new Shader(
        SOURCE_DIR "/resources/shader/playerBox.vs",
        SOURCE_DIR "/resources/shader/playerBox.fs"
    );

    // Box from (-0.3, 0, -0.3) to (0.3, 1.75, 0.3)
    float vertices[] = {
        // Front face
        -0.3f, 0.0f,  0.3f,
         0.3f, 0.0f,  0.3f,
         0.3f, 1.75f, 0.3f,
        -0.3f, 1.75f, 0.3f,
        // Back face
        -0.3f, 0.0f, -0.3f,
         0.3f, 0.0f, -0.3f,
         0.3f, 1.75f,-0.3f,
        -0.3f, 1.75f,-0.3f,
    };

    GLuint indices[] = {
        // Front
        0, 1, 2, 2, 3, 0,
        // Back
        5, 4, 7, 7, 6, 5,
        // Left
        4, 0, 3, 3, 7, 4,
        // Right
        1, 5, 6, 6, 2, 1,
        // Top
        3, 2, 6, 6, 7, 3,
        // Bottom
        4, 5, 1, 1, 0, 4,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

PlayerBoxRenderer::~PlayerBoxRenderer() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    delete shader;
}

void PlayerBoxRenderer::render(const std::unordered_map<uint32_t, RemotePlayer>& players,
                               const glm::mat4& view, const glm::mat4& proj) {
    if (players.empty()) return;

    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", proj);
    shader->setVec3("playerColor", glm::vec3(0.2f, 0.6f, 1.0f));

    glBindVertexArray(vao);
    for (const auto& pair : players) {
        const RemotePlayer& rp = pair.second;
        glm::mat4 model = glm::translate(glm::mat4(1.0f), rp.position - glm::vec3(0.0f, 1.75f, 0.0f));
        shader->setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);
}
