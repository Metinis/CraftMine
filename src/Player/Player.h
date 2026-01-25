#ifndef CRAFTMINE_PLAYER_H
#define CRAFTMINE_PLAYER_H
#pragma once
#include "Input/Camera.h"
#include "WorldGen/Chunk.h"
#include "WorldGen/World.h"
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
    World* world{};
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
    void calculateNewPositionY(const float& deltaTime);
    void UpdatePositionXZ(const glm::vec3& newPosition);
    void UpdateDeceleration(const float& deltaTime);
    void ProcessKeyboardMovement(cameraMovement dir, float deltaTime);
    bool isColliding(glm::vec3 newPosition) const;
    bool isHeadInWater() const;
    bool checkNewPositionZ(float newZ) const;
    bool checkNewPositionX(float newX) const;
    bool checkNewPositionXZ(const glm::vec3& newPosition) const;
    bool checkNewPositionY(const glm::vec3& newPosition) const;
    void applyNewPositionY(const glm::vec3& newPosition);
    void updateShifting();
    glm::vec3 positionInChunk() const;
    bool checkCollisionWithBlockLocal(glm::ivec3 localPos) const;
    void setBlockID(int blockID);
    int getBlockID() const;

    void updateFlying();

    bool isFlying{};

    void checkIfSwimming(glm::ivec3 pos);

    bool loadPlayerPosFromFile();
    void savePosToFile() const;
};


#endif //CRAFTMINE_PLAYER_H
