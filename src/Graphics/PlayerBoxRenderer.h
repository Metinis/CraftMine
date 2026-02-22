#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <cstdint>

class Shader;
struct RemotePlayer;

class PlayerBoxRenderer {
public:
    PlayerBoxRenderer();
    ~PlayerBoxRenderer();

    void render(const std::unordered_map<uint32_t, RemotePlayer>& players,
                const glm::mat4& view, const glm::mat4& proj);

private:
    Shader* shader;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
};
