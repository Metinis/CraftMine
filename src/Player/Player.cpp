#include "Player.h"
Player::Player(){
    movementSpeed = 17.5f;
    position = glm::vec3(16.0f, 20.0f, 16.0f);
    camera.position = &position;
}
void Player::ProcessKeyboardMovement(cameraMovement dir, float deltaTime)
{
    glm::ivec3 globalPos;

    globalPos.x = static_cast<int>(std::round(position.x));
    globalPos.y = static_cast<int>(std::round(position.y));
    globalPos.z = static_cast<int>(std::round(position.z));

    chunkPosition.x = static_cast<int>(globalPos.x / Chunk::SIZE);
    chunkPosition.y = static_cast<int>(globalPos.z / Chunk::SIZE);

    Chunk* currentChunk = world->GetChunk(chunkPosition.x, chunkPosition.y);


    glm::vec3 newChunkPos;
    glm::ivec3 localChunkPos; //position in chunk
    localChunkPos.x = globalPos.x - currentChunk->chunkPosition.x * Chunk::SIZE;
    localChunkPos.y = globalPos.y;
    localChunkPos.z = globalPos.z - currentChunk->chunkPosition.y * Chunk::SIZE;


    //newChunkPos.y = localChunkPos.y;

    float velocity = movementSpeed * deltaTime;

    //TODO fix chunk borders on width calculations00
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

        //if(!isColliding(newPosition, camera.Front))
        //position += glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * velocity;
        if(!isColliding(newPosition, camera.Front))
        {
            position -= glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * velocity;
        }
        else if(!checkNewPositionX(camera.Front, newPosition) && checkNewPositionZ(camera.Front, newPosition))
        {
            position.x -= (glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * velocity).x;
        }
        else if(!checkNewPositionZ(camera.Front, newPosition) && checkNewPositionX(camera.Front, newPosition))
        {
            position.z -= (glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * velocity).z;
        }
    }
    if (dir == cameraMovement::LEFT) {
        glm::vec3 newPosition = position - camera.Right * velocity;

        if(!isColliding(newPosition, camera.Front))
        {
            position -= glm::normalize(camera.Right) * velocity;
        }
        else if(!checkNewPositionX(camera.Front, newPosition) && checkNewPositionZ(camera.Front, newPosition))
        {
            position.x -= (glm::normalize(camera.Right) * velocity).x;
        }
        else if(!checkNewPositionZ(camera.Front, newPosition) && checkNewPositionX(camera.Front, newPosition))
        {
            position.z -= (glm::normalize(camera.Right) * velocity).z;
        }
    }
    if (dir == cameraMovement::RIGHT) {
        glm::vec3 newPosition = position + camera.Right * velocity;

        //if(!isColliding(newPosition, camera.Right))
        //    position += glm::normalize(camera.Right) * velocity;
        if(!isColliding(newPosition, camera.Front))
        {
            position += glm::normalize(camera.Right) * velocity;
        }
        else if(!checkNewPositionX(camera.Front, newPosition) && checkNewPositionZ(camera.Front, newPosition))
        {
            position.x += (glm::normalize(camera.Right) * velocity).x;
        }
        else if(!checkNewPositionZ(camera.Front, newPosition) && checkNewPositionX(camera.Front, newPosition))
        {
            position.z += (glm::normalize(camera.Right) * velocity).z;
        }
    }
    if (dir == cameraMovement::DOWN) {
        if(currentChunk->GetBlockID(glm::round(glm::vec3(localChunkPos.x, position.y - velocity - 1.5, localChunkPos.z))) == 0 &&
           currentChunk->GetBlockID(glm::round(glm::vec3(localChunkPos.x, position.y - velocity - 0.5, localChunkPos.z))) == 0)
            position.y -= velocity;
    }
    if (dir == cameraMovement::UP) {
        if(currentChunk->GetBlockID(glm::round(glm::vec3(localChunkPos.x, position.y - velocity + 0.4, localChunkPos.z))) == 0)
            position.y += velocity;
    }
}
bool Player::isColliding(glm::vec3& newPosition, glm::vec3 front) //front can be x or z since they are checked separately
{
    //by default false
    return (checkNewPositionZ(front, newPosition) ||
            checkNewPositionX(front, newPosition)) ||
           checkNewPositionXZ(front, newPosition);

}
bool Player::checkNewPositionZ(glm::vec3 front, glm::vec3 newPosition)
{
    Chunk* currentChunk;
    glm::vec3 newChunkPos;
    glm::ivec2 _chunkPosition;
    float _width = (newPosition.z > position.z) ? width : -width;
    _chunkPosition.x = static_cast<int>(glm::round(position.x) / Chunk::SIZE);
    _chunkPosition.y = static_cast<int>(glm::round(newPosition.z + _width) / Chunk::SIZE);

    currentChunk = world->GetChunk(_chunkPosition.x, _chunkPosition.y);

    glm::vec3 localChunkPos; //position in chunk
    localChunkPos.x = glm::round((position.x) - chunkPosition.x * Chunk::SIZE);
    localChunkPos.y = glm::round(position.y);
    localChunkPos.z = glm::round((position.z) - chunkPosition.y * Chunk::SIZE);
    localChunkPos.x = (localChunkPos.x >= 16) ? 0 : localChunkPos.x;
    localChunkPos.x = (localChunkPos.x < 0) ? 15 : localChunkPos.x;
    int xPlus = (glm::round(position.x + width) - chunkPosition.x * Chunk::SIZE);
    int xMinus = (glm::round(position.x - width) - chunkPosition.x * Chunk::SIZE);
    if(localChunkPos.x > 15 || localChunkPos.x < 0)
    {
        return true;
    }

    newChunkPos.z = (newPosition.z - currentChunk->chunkPosition.y * Chunk::SIZE);
    newChunkPos.z = glm::round(newChunkPos.z + _width);

    if(currentChunk->GetBlockID(glm::vec3(xPlus, localChunkPos.y, newChunkPos.z)) != 0 ||
       currentChunk->GetBlockID(glm::vec3(xPlus, localChunkPos.y-1, newChunkPos.z)) != 0 ||
       currentChunk->GetBlockID(glm::vec3(xMinus, localChunkPos.y, newChunkPos.z)) != 0 ||
       currentChunk->GetBlockID(glm::vec3(xMinus, localChunkPos.y-1, newChunkPos.z)) != 0)
    {
        return true;
    }
    return false;
}
bool Player::checkNewPositionX(glm::vec3 front, glm::vec3 newPosition)
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
    int zPlus = (glm::round(position.z + width) - chunkPosition.y * Chunk::SIZE);
    int zMinus = (glm::round(position.z - width) - chunkPosition.y * Chunk::SIZE);
    localChunkPos.z = (glm::round(position.z) - chunkPosition.y * Chunk::SIZE);
    localChunkPos.z = (localChunkPos.z >= 16) ? 0 : localChunkPos.z;
    localChunkPos.z = (localChunkPos.z < 0) ? 15 : localChunkPos.z;
    if(localChunkPos.z > 15 || localChunkPos.z < 0)
    {
        return true;
    }

    newChunkPos.x = (newPosition.x - currentChunk->chunkPosition.x * Chunk::SIZE);
    newChunkPos.x = glm::round(newChunkPos.x + _width);

    if(currentChunk->GetBlockID(glm::vec3(newChunkPos.x, localChunkPos.y, zPlus)) != 0 ||
       currentChunk->GetBlockID(glm::vec3(newChunkPos.x, localChunkPos.y-1, zPlus)) != 0 ||
       currentChunk->GetBlockID(glm::vec3(newChunkPos.x, localChunkPos.y, zMinus)) != 0 ||
       currentChunk->GetBlockID(glm::vec3(newChunkPos.x, localChunkPos.y-1, zMinus)) != 0)
    {
        return true;
    }
    return false;
}
bool Player::checkNewPositionXZ(glm::vec3 front, glm::vec3 newPosition)
{
    Chunk* currentChunk;
    glm::vec3 newChunkPos;
    glm::ivec2 _chunkPosition;
    float _widthZ = (newPosition.z > position.z) ? width : -width;
    float _widthX = (newPosition.x > position.x) ? width : -width;
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
    newChunkPos.x = glm::round(newChunkPos.x + _widthX);
    newChunkPos.z = glm::round(newChunkPos.z + _widthZ);

    if(currentChunk->GetBlockID(glm::vec3(newChunkPos.x, newChunkPos.y, newChunkPos.z)))
    {
        return true;
    }
    return false;
}


