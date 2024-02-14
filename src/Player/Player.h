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
    float width = 0.6f;
    float movementSpeed;
public:
    World* world;
    Camera camera;
    glm::vec3 position{};
    glm::ivec2 chunkPosition{};
    Player(){
        movementSpeed = 17.5f;
        position = glm::vec3(16.0f, 20.0f, 16.0f);
        camera.position = &position;
    }
    void ProcessKeyboardMovement(cameraMovement dir, float deltaTime)
    {
        glm::ivec3 globalPos;

        globalPos.x = static_cast<int>(std::round(position.x));
        globalPos.y = static_cast<int>(std::round(position.y));
        globalPos.z = static_cast<int>(std::round(position.z));

        chunkPosition.x = static_cast<int>(position.x / Chunk::SIZE);
        chunkPosition.y = static_cast<int>(position.z / Chunk::SIZE);

        Chunk* currentChunk = world->GetChunk(chunkPosition.x, chunkPosition.y);


        glm::vec3 newChunkPos;
        glm::ivec3 localChunkPos; //position in chunk
        localChunkPos.x = position.x - currentChunk->chunkPosition.x * Chunk::SIZE;
        localChunkPos.y = position.y;
        localChunkPos.z = position.z - currentChunk->chunkPosition.y * Chunk::SIZE;


        //newChunkPos.y = localChunkPos.y;

        float velocity = movementSpeed * deltaTime;

        //TODO add width into account to individual checks
        if (dir == cameraMovement::FORWARD){
            glm::vec3 newPosition = position + glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * velocity;

            //if(!isColliding(newPosition, camera.Front))
            //position += glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * velocity;
            if(!isColliding(newPosition, camera.Front))
            {
                position += glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * velocity;
            }
            else if(!checkNewPositionX(camera.Front, newPosition) && checkNewPositionZ(camera.Front, newPosition))
            {
                position.x += (glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * velocity).x;
            }
            else if(!checkNewPositionZ(camera.Front, newPosition) && checkNewPositionX(camera.Front, newPosition))
            {
                position.z += (glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * velocity).z;
            }
        }
        if (dir == cameraMovement::BACKWARD) {
            glm::vec3 newPosition = position - glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * velocity;

            if(!isColliding(newPosition, glm::vec3(-camera.Front.x, 0, -camera.Front.z)))
                position -= glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * velocity;
        }
        if (dir == cameraMovement::LEFT) {
            glm::vec3 newPosition = position - camera.Right * velocity;

            if(!isColliding(newPosition, -camera.Right))
                position -= glm::normalize(camera.Right) * velocity;
        }
        if (dir == cameraMovement::RIGHT) {
            glm::vec3 newPosition = position + camera.Right * velocity;

            if(!isColliding(newPosition, camera.Right))
                position += glm::normalize(camera.Right) * velocity;
        }
        if (dir == cameraMovement::DOWN) {
            if(currentChunk->GetBlockID(glm::round(glm::vec3(localChunkPos.x, position.y - velocity, localChunkPos.z))) == 0)
                position.y -= velocity;
        }
        if (dir == cameraMovement::UP) {
            if(currentChunk->GetBlockID(glm::round(glm::vec3(localChunkPos.x, position.y + velocity, localChunkPos.z))) == 0)
                position.y += velocity;
        }
    }
    bool isColliding(glm::vec3& newPosition, glm::vec3 front) //front can be x or z since they are checked separately
    {
        //by default false
        return (checkNewPositionZ(front, newPosition) ||
                checkNewPositionX(front, newPosition)) ||
                checkNewPositionXZ(front, newPosition);

    }
    bool checkNewPositionZ(glm::vec3 front, glm::vec3 newPosition)
    {
        Chunk* currentChunk;
        glm::vec3 newChunkPos;
        glm::ivec2 _chunkPosition;
        float _width = (newPosition.z > position.z) ? 1 - width : width-1;
        _chunkPosition.x = static_cast<int>(glm::round(position.x) / Chunk::SIZE);
        _chunkPosition.y = static_cast<int>(glm::round(newPosition.z + _width) / Chunk::SIZE);

        currentChunk = world->GetChunk(_chunkPosition.x, _chunkPosition.y);

        glm::vec3 localChunkPos; //position in chunk
        localChunkPos.x = glm::round((position.x) - chunkPosition.x * Chunk::SIZE);
        localChunkPos.y = glm::round(position.y);
        localChunkPos.z = glm::round((position.z) - chunkPosition.y * Chunk::SIZE);
        localChunkPos.x = (localChunkPos.x >= 16) ? 0 : localChunkPos.x;
        localChunkPos.x = (localChunkPos.x < 0) ? 15 : localChunkPos.x;
        if(localChunkPos.x > 15 || localChunkPos.x < 0)
        {
            return true;
        }

        newChunkPos.z = (newPosition.z - currentChunk->chunkPosition.y * Chunk::SIZE);
        newChunkPos.z = glm::round(_width + newChunkPos.z);

        if(currentChunk->chunkPosition.x != chunkPosition.x || currentChunk->chunkPosition.y != chunkPosition.y)
        {
            std::cout<<currentChunk->chunkPosition.x<<"xx "<<currentChunk->chunkPosition.y<<"yy \n";
            std::cout<<chunkPosition.x<<"xx "<<chunkPosition.y<<"yy \n";

        }
        if(currentChunk->GetBlockID(glm::vec3(localChunkPos.x, localChunkPos.y, newChunkPos.z)) != 0)
        {
            return true;
        }
        return false;
    }
    bool checkNewPositionX(glm::vec3 front, glm::vec3 newPosition)
    {
        Chunk* currentChunk;
        glm::vec3 newChunkPos;
        glm::ivec2 _chunkPosition;
        float _width = (newPosition.x > position.x) ? width : -width;
        _chunkPosition.x = static_cast<int>(glm::round(newPosition.x + _width) / Chunk::SIZE);
        _chunkPosition.y = static_cast<int>(glm::round(position.z) / Chunk::SIZE);

        currentChunk = world->GetChunk(_chunkPosition.x, _chunkPosition.y);

        glm::vec3 localChunkPos; //position in chunk
        localChunkPos.x = glm::round((position.x) - chunkPosition.x * Chunk::SIZE);
        localChunkPos.y = glm::round(position.y);
        localChunkPos.z = (glm::round(position.z) - chunkPosition.y * Chunk::SIZE);
        localChunkPos.z = (localChunkPos.z >= 16) ? 0 : localChunkPos.z;
        localChunkPos.z = (localChunkPos.z < 0) ? 15 : localChunkPos.z;
        if(localChunkPos.z > 15 || localChunkPos.z < 0)
        {
            return true;
        }

        newChunkPos.x = (newPosition.x - currentChunk->chunkPosition.x * Chunk::SIZE);
        newChunkPos.x = glm::round(_width + newChunkPos.x);

        if(currentChunk->GetBlockID(glm::vec3(newChunkPos.x, localChunkPos.y, localChunkPos.z)) != 0)
        {
            return true;
        }
        return false;
    }
    bool checkNewPositionXZ(glm::vec3 front, glm::vec3 newPosition)
    {
        Chunk* currentChunk;
        glm::vec3 newChunkPos;
        glm::ivec2 _chunkPosition;
        float _widthZ = (newPosition.z > position.z) ? 1 - width : width - 1;
        float _widthX = (newPosition.x > position.x) ? 1 - width : width - 1;
        _chunkPosition.x = static_cast<int>(glm::round(newPosition.x + _widthX) / Chunk::SIZE);
        _chunkPosition.y = static_cast<int>(glm::round(newPosition.z + _widthZ) / Chunk::SIZE);
        currentChunk = world->GetChunk(_chunkPosition.x, _chunkPosition.y);

        glm::vec3 localChunkPos; //position in chunk
        localChunkPos.x = glm::round((position.x) - chunkPosition.x * Chunk::SIZE);
        localChunkPos.y = glm::round(position.y);
        localChunkPos.z = glm::round((position.z) - chunkPosition.y * Chunk::SIZE);
        localChunkPos.x = (localChunkPos.x >= 16) ? 0 : localChunkPos.x;
        localChunkPos.x = (localChunkPos.x < 0) ? 15 : localChunkPos.x;
        localChunkPos.z = (localChunkPos.z >= 16) ? 0 : localChunkPos.z;
        localChunkPos.z = (localChunkPos.z < 0) ? 15 : localChunkPos.z;

        newChunkPos.x = (newPosition.x - currentChunk->chunkPosition.x * Chunk::SIZE);
        newChunkPos.y = glm::round(localChunkPos.y);
        newChunkPos.z = (newPosition.z - currentChunk->chunkPosition.y * Chunk::SIZE);
        newChunkPos.x = glm::round(_widthX + newChunkPos.x);
        newChunkPos.z = glm::round(_widthZ + newChunkPos.z);

        if(currentChunk->GetBlockID(glm::vec3(newChunkPos)) != 0)
        {
            return true;
        }
        return false;
    }
};


#endif //CRAFTMINE_PLAYER_H
