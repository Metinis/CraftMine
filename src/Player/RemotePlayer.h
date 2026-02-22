#pragma once
#include <string>
#include <cstdint>
#include <glm/glm.hpp>

struct RemotePlayer {
    uint32_t id;
    std::string username;
    glm::vec3 position;
    glm::vec3 lastPosition;
    float yaw;
    float pitch;
    float interpTimer;

    RemotePlayer() : id(0), yaw(0), pitch(0), interpTimer(0) {}
    RemotePlayer(uint32_t id, const std::string& username, glm::vec3 pos, float yaw, float pitch)
        : id(id), username(username), position(pos), lastPosition(pos), yaw(yaw), pitch(pitch), interpTimer(0) {}
};
