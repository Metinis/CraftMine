#include "Player.h"
Player::Player(){
    movementSpeed = 5.0f;
    position = glm::vec3(16.0f, 20.0f, 16.0f);
    camera.position = &position;
}
void Player::Update(float deltaTime){

    UpdatePositionGravity(deltaTime);

    if(isJumping && playerVelocity.y <= 0)
    {
        isJumping = false;
    }

    UpdatePositionXZ(deltaTime);

    UpdateDeceleration(deltaTime);

}
void Player::UpdatePositionGravity(float& deltaTime) {
    //TODO add 2 more current chunk refs for edges
    glm::ivec3 globalPos;

    globalPos.x = static_cast<int>(std::round(position.x));
    globalPos.y = static_cast<int>(std::round(position.y));
    globalPos.z = static_cast<int>(std::round(position.z));

    chunkPosition.x = static_cast<int>(globalPos.x / Chunk::SIZE);
    chunkPosition.y = static_cast<int>(globalPos.z / Chunk::SIZE);

    Chunk* currentChunk = world->GetChunk(chunkPosition.x, chunkPosition.y);

    glm::vec3 localChunkPos; //position in chunk
    localChunkPos.x = (position.x - (float)currentChunk->chunkPosition.x * Chunk::SIZE);
    localChunkPos.y = (position.y);
    localChunkPos.z = (position.z - (float)currentChunk->chunkPosition.y * Chunk::SIZE);

    int xPos = (int)glm::round(localChunkPos.x + WIDTH);
    int xNeg = (int)glm::round(localChunkPos.x - WIDTH);
    int zPos = (int)glm::round(localChunkPos.z + WIDTH);
    int zNeg = (int)glm::round(localChunkPos.z - WIDTH);

    if(currentChunk->GetBlockID(glm::round(glm::vec3(xPos, position.y - HEIGHT, zPos))) == 0 &&
       currentChunk->GetBlockID(glm::round(glm::vec3(xPos, position.y - HEIGHT, zNeg))) == 0 &&
       currentChunk->GetBlockID(glm::round(glm::vec3(xNeg, position.y - HEIGHT, zPos))) == 0 &&
       currentChunk->GetBlockID(glm::round(glm::vec3(xNeg, position.y - HEIGHT, zNeg))) == 0 && !isJumping)
    {
        isGrounded = false;
    }
    else if(!isJumping && !isGrounded){
        isGrounded = true;
        playerVelocity.y = 0;
    }

    if(!isGrounded)
    {
        if(playerVelocity.y > -MAX_VELOCITY)
            playerVelocity.y -= GRAVITY * deltaTime * GRAVITY_MULTIPLIER;
    }
}
void Player::UpdatePositionXZ(float &deltaTime) {
    glm::vec3 newPosition = position + playerVelocity * deltaTime;// * deltaTime;
    position.y = newPosition.y;
    if(!isColliding(newPosition, camera.Front))
    {
        position.x = newPosition.x;
        position.z = newPosition.z;
    }
    else{
        glm::vec2 newVelocity;
        if(!checkNewPositionX(newPosition.x))
        {
            position.x = newPosition.x;
        }
        else
        {
            playerVelocity.x = 0;
        }
        if(!checkNewPositionZ(newPosition.z))
        {
            position.z = newPosition.z;
        }
        else
        {
            playerVelocity.z = 0;
        }
    }
}
void Player::UpdateDeceleration(float &deltaTime){
    // Calculate the magnitude of the velocity vector in the XZ plane
    float velocityMagnitudeXZ = glm::length(glm::vec2(playerVelocity.x, playerVelocity.z));

    // Apply deceleration only to the X and Z components
    if (velocityMagnitudeXZ > 0) {
        // Calculate the deceleration amount based on your chosen factor (e.g., 25)
        float deceleration = 25 * deltaTime;

        // Ensure the deceleration doesn't overshoot zero
        velocityMagnitudeXZ = glm::max(0.0f, velocityMagnitudeXZ - deceleration);

        // Normalize the X and Z components and multiply by the new magnitude
        glm::vec2 velocityXZ = glm::normalize(glm::vec2(playerVelocity.x, playerVelocity.z)) * velocityMagnitudeXZ;

        // Update the playerVelocity vector with the new X and Z components, keeping the Y component unchanged
        playerVelocity = glm::vec3(velocityXZ.x, playerVelocity.y, velocityXZ.y);
    } else {
        playerVelocity.x = 0.0f;
        playerVelocity.z = 0.0f;
    }
}
void Player::ProcessKeyboardMovement(cameraMovement dir, float deltaTime)
{
    glm::vec2 playerVelocityXZ;
    playerVelocityXZ.x = playerVelocity.x;
    playerVelocityXZ.y = playerVelocity.z;

    if (dir == cameraMovement::FORWARD){
        if(glm::length(playerVelocityXZ) < movementSpeed){
            playerVelocity += glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * deltaTime * ACCELERATION;
        }


    }
    if (dir == cameraMovement::BACKWARD) {
        if(glm::length(playerVelocityXZ) < movementSpeed){
            playerVelocity -= glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * deltaTime * ACCELERATION;
        }

    }
    if (dir == cameraMovement::LEFT) {
        //if(glm::length(playerVelocityXZ) < movementSpeed){
            playerVelocity -= camera.Right * deltaTime * ACCELERATION;
        //}


    }
    if (dir == cameraMovement::RIGHT) {
        //if(glm::length(playerVelocityXZ) < movementSpeed){
            playerVelocity += camera.Right * deltaTime * ACCELERATION;
        //}
    }
    if (dir == cameraMovement::DOWN) {
        //TODO Implement shifting
    }
    if (dir == cameraMovement::UP) {
        if(isGrounded && !isJumping)
        {
            playerVelocity.y = 0;
            isGrounded = false;
            isJumping = true;
            playerVelocity.y += 6.5;
        }

    }
}
bool Player::isColliding(glm::vec3& newPosition, glm::vec3 front) const //front can be x or z since they are checked separately
{
    //by default false
    return (checkNewPositionZ(newPosition.z) ||
            checkNewPositionX(newPosition.x)) ||
           checkNewPositionXZ(newPosition);

}
bool Player::checkNewPositionZ(float newZ) const
{
    Chunk* currentChunkMinus;
    Chunk* currentChunkPlus;
    glm::ivec2 _chunkPosition;
    float _width = (newZ > position.z) ? WIDTH + 0.01f : -WIDTH - 0.01f;

    _chunkPosition.x = (int)(glm::round(position.x - WIDTH) / Chunk::SIZE);
    _chunkPosition.y = (int)(glm::round(newZ + _width) / Chunk::SIZE);

    currentChunkMinus = world->GetChunk(_chunkPosition.x, _chunkPosition.y);

    _chunkPosition.x = (int)(glm::round(position.x + WIDTH) / Chunk::SIZE);
    _chunkPosition.y = (int)(glm::round(newZ + _width) / Chunk::SIZE);

    currentChunkPlus = world->GetChunk(_chunkPosition.x, _chunkPosition.y);

    int y1 = (int)glm::round(position.y);
    int y2 = (int)glm::round(position.y - 1.5);
    int xPlus = (int)(glm::round(position.x + WIDTH) - (float)currentChunkPlus->chunkPosition.x * Chunk::SIZE);
    int xMinus = (int)(glm::round(position.x - WIDTH) - (float)currentChunkMinus->chunkPosition.x * Chunk::SIZE);

    int newChunkPosPlus = (int)glm::round((newZ - (float)currentChunkPlus->chunkPosition.y * Chunk::SIZE) + _width);
    int newChunkPosMinus = (int)glm::round((newZ - (float)currentChunkMinus->chunkPosition.y * Chunk::SIZE) + _width);

    if(currentChunkPlus->GetBlockID(glm::vec3(xPlus, y1, newChunkPosPlus)) != 0 ||
       currentChunkPlus->GetBlockID(glm::vec3(xPlus, y2, newChunkPosPlus)) != 0 ||
       currentChunkMinus->GetBlockID(glm::vec3(xMinus, y1, newChunkPosMinus)) != 0 ||
       currentChunkMinus->GetBlockID(glm::vec3(xMinus, y2, newChunkPosMinus)) != 0)
    {
        return true;
    }
    return false;
}
bool Player::checkNewPositionX(float newX) const
{
    Chunk* currentChunkMinus;
    Chunk* currentChunkPlus;
    glm::vec3 newChunkPos;
    glm::ivec2 _chunkPosition;
    float _width = (newX > position.x) ? WIDTH + 0.01f : -WIDTH - 0.01f;

    _chunkPosition.x = (int)(glm::round(newX + _width) / Chunk::SIZE);
    _chunkPosition.y = (int)(glm::round(position.z - WIDTH) / Chunk::SIZE);

    currentChunkMinus = world->GetChunk(_chunkPosition.x, _chunkPosition.y);

    _chunkPosition.x = (int)(glm::round(newX + _width) / Chunk::SIZE);
    _chunkPosition.y = (int)(glm::round(position.z + WIDTH) / Chunk::SIZE);

    currentChunkPlus = world->GetChunk(_chunkPosition.x, _chunkPosition.y);

    int y1 = (int)glm::round(position.y);
    int y2 = (int)glm::round(position.y - 1.5);
    int zPlus = (int)(glm::round(position.z + WIDTH) - (float)currentChunkPlus->chunkPosition.y * Chunk::SIZE);
    int zMinus = (int)(glm::round(position.z - WIDTH) - (float)currentChunkMinus->chunkPosition.y * Chunk::SIZE);

    int newChunkPosPlus = (int)glm::round((newX - (float)currentChunkPlus->chunkPosition.x * Chunk::SIZE) + _width);
    int newChunkPosMinus = (int)glm::round((newX - (float)currentChunkMinus->chunkPosition.x * Chunk::SIZE) + _width);

    if(currentChunkPlus->GetBlockID(glm::vec3(newChunkPosPlus, y1, zPlus)) != 0 ||
       currentChunkPlus->GetBlockID(glm::vec3(newChunkPosPlus, y2, zPlus)) != 0 ||
       currentChunkMinus->GetBlockID(glm::vec3(newChunkPosMinus, y1, zMinus)) != 0 ||
       currentChunkMinus->GetBlockID(glm::vec3(newChunkPosMinus, y2, zMinus)) != 0)
    {
        return true;
    }
    return false;
}
bool Player::checkNewPositionXZ(glm::vec3 newPosition) const
{
    Chunk* currentChunk;
    glm::vec3 newChunkPos;
    glm::ivec2 _chunkPosition;

    float _widthZ = (newPosition.z > position.z) ? WIDTH + 0.01f : -WIDTH - 0.01f;
    float _widthX = (newPosition.x > position.x) ? WIDTH + 0.01f : -WIDTH - 0.01f;

    _chunkPosition.x = static_cast<int>(glm::round(newPosition.x + _widthX) / Chunk::SIZE);
    _chunkPosition.y = static_cast<int>(glm::round(newPosition.z + _widthZ) / Chunk::SIZE);
    currentChunk = world->GetChunk(_chunkPosition.x, _chunkPosition.y);

    newChunkPos.x = ((newPosition.x - (float)currentChunk->chunkPosition.x * Chunk::SIZE) + _widthX);
    newChunkPos.y = glm::round(position.y);
    newChunkPos.z = ((newPosition.z - (float)currentChunk->chunkPosition.y * Chunk::SIZE)+ _widthZ);

    if(currentChunk->GetBlockID(glm::vec3(newChunkPos.x, newChunkPos.y, newChunkPos.z)))
    {
        return true;
    }
    return false;
}


