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
    const float GRAVITY = 9.8f;
    const float HEIGHT = 1.75f;
    const float WIDTH = 0.35f;
    const float GRAVITY_MULTIPLIER = 2;
    const float MAX_VELOCITY = 40.0f;
    const float ACCELERATION = 50.0f;
    const float DECELERATION = 25.0f;

    bool isGrounded{};
    bool isJumping{};

    float movementSpeed;

    glm::vec3 playerVelocity{};
public:
    World* world{};
    Camera camera;
    glm::vec3 position{};

    Player();
    void Update(float deltaTime);
    void UpdatePositionY(float& deltaTime, glm::vec3& newPosition);
    void UpdatePositionXZ(float& deltaTime, glm::vec3& newPosition);
    void UpdateDeceleration(float& deltaTime);
    void ProcessKeyboardMovement(cameraMovement dir, float deltaTime);
    bool isColliding(glm::vec3& newPosition, glm::vec3 front) const;
    bool checkNewPositionZ(float newZ) const;
    bool checkNewPositionX(float newZ) const;
    bool checkNewPositionXZ(glm::vec3 newPosition) const;
    bool checkNewPositionY(float newY) const;
};


#endif //CRAFTMINE_PLAYER_H
