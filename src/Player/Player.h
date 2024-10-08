#ifndef CRAFTMINE_PLAYER_H
#define CRAFTMINE_PLAYER_H
#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include "Input/Camera.h"
#include "Chunk.h"
#include "World.h"
#include "Inventory.h"

class Player {
private:
    const float GRAVITY = 9.8f;
    float HEIGHT = 1.75f;
    const float WIDTH = 0.3f;
    const float GRAVITY_MULTIPLIER = 2;
    const float MAX_VELOCITY = 40.0f;
    float ACCELERATION = 50.0f;
    const float DECELERATION = 25.0f;
    float jumpForce = 6.2f;


    int currentBlockID = 1;

    float movementSpeed;
    //todo fix private and public attributes

public:
    World* world;
    Camera camera;
    glm::vec3 position{};
    glm::vec3 lastPosition{};
    glm::ivec2 chunkPosition{};

    Toolbar* toolbar;
    Inventory* inventory;

    bool shiftChanged = false;
    bool isJumping{};
    bool isGrounded{};
    bool isShifting{};
    bool isSwimming{};
    glm::vec3 playerVelocity{};

    Player();
    void Update(float deltaTime);
    void calculateNewPositionY(float& deltaTime);
    void UpdatePositionXZ(glm::vec3& newPosition);
    void UpdateDeceleration(float& deltaTime);
    void ProcessKeyboardMovement(cameraMovement dir, float deltaTime);
    bool isColliding(glm::vec3 newPosition) const;
    bool isHeadInWater();
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

    void updateFlying();

    bool isFlying{};

    void checkIfSwimming(glm::ivec3 pos);

    bool loadPlayerPosFromFile();
    void savePosToFile();
};


#endif //CRAFTMINE_PLAYER_H
