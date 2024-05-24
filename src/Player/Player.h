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
    float HEIGHT = 1.55f;
    const float WIDTH = 0.3f;
    const float GRAVITY_MULTIPLIER = 2;
    const float MAX_VELOCITY = 40.0f;
    const float ACCELERATION = 50.0f;
    const float DECELERATION = 25.0f;



    bool isFlying{};

    int currentBlockID = 1;

    float movementSpeed;
    //todo fix private and public attributes

public:
    World* world;
    Camera camera;
    glm::vec3 position{};
    glm::vec3 lastPosition{};
    glm::ivec2 chunkPosition{};

    bool shiftChanged = true;
    bool isJumping{};
    bool isGrounded{};
    bool isShifting{};
    glm::vec3 playerVelocity{};

    Player();
    void Update(float deltaTime);
    void calculateNewPositionY(float& deltaTime);
    void UpdatePositionXZ(glm::vec3& newPosition);
    void UpdateDeceleration(float& deltaTime);
    void ProcessKeyboardMovement(cameraMovement dir, float deltaTime);
    bool isColliding(glm::vec3& newPosition, glm::vec3 front) const;
    bool checkNewPositionZ(float newZ) const;
    bool checkNewPositionX(float newZ) const;
    bool checkNewPositionXZ(glm::vec3 newPosition) const;
    bool checkNewPositionY(glm::vec3& newPosition) const;
    void applyNewPositionY(glm::vec3& newPosition);
    void updateShifting();
    glm::vec3 positionInChunk();
    bool checkCollisionWithBlockLocal(glm::ivec3 localPos);
    void setBlockID(int blockID);
    int getBlockID();
};


#endif //CRAFTMINE_PLAYER_H
