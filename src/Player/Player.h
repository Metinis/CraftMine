#ifndef CRAFTMINE_PLAYER_H
#define CRAFTMINE_PLAYER_H
#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include "Input/Camera.h"
#include "Chunk.h"
#include "World.h"

class Player {
private:
    float width = 0.35f;
    float movementSpeed;
public:
    World* world;
    Camera camera;
    glm::vec3 position{};
    glm::ivec2 chunkPosition{};
    Player();
    void ProcessKeyboardMovement(cameraMovement dir, float deltaTime);
    bool isColliding(glm::vec3& newPosition, glm::vec3 front);
    bool checkNewPositionZ(glm::vec3 front, glm::vec3 newPosition);
    bool checkNewPositionX(glm::vec3 front, glm::vec3 newPosition);
    bool checkNewPositionXZ(glm::vec3 front, glm::vec3 newPosition);
};


#endif //CRAFTMINE_PLAYER_H
