#include "Player.h"
Player::Player(){
    movementSpeed = 5.0f;
    position = glm::vec3(World::SIZE*Chunk::SIZE / 2, Chunk::HEIGHT, World::SIZE*Chunk::SIZE / 2);
    camera.position = &position;
    chunkPosition = glm::vec2(position.x / Chunk::SIZE, position.z / Chunk::SIZE);
}
void Player::Update(float deltaTime){

    lastPosition = position;
    glm::vec3 newPosition = position + playerVelocity * deltaTime;

    UpdatePositionY(deltaTime, newPosition);
    if(glm::length(playerVelocity) != 0 || !isGrounded){
        UpdatePositionXZ(newPosition);
        //Check for sorting every time you move
        SortTransparentFaces();
    }

    if(isJumping && playerVelocity.y <= 0)
    {
        isJumping = false;
    }

    if(glm::length(playerVelocity) > 0)
        UpdateDeceleration(deltaTime);

    if(isShifting && !shiftChanged){
        position.y -= 0.2f;
        HEIGHT = 1.73f;
        movementSpeed = 2.5f;
        shiftChanged = true;
    }
    else if(!isShifting && shiftChanged)
    {
        position.y += 0.2f;
        HEIGHT = 1.75f;
        movementSpeed = 5.0f;
        shiftChanged = false;
    }
}
void Player::UpdatePositionY(float& deltaTime, glm::vec3& newPosition) {

    glm::vec3 newYPos = glm::vec3(position.x, newPosition.y + HEIGHT + 0.2f, position.z);
    if(!checkNewPositionY(newYPos)) {
        position.y = newPosition.y;
    }
    else if(!isGrounded){
        playerVelocity.y = 0;
        isJumping = false;
    }

    isGrounded = checkNewPositionY(position);
    if(!isJumping && isGrounded){
        playerVelocity.y = 0;
    }
    if (!isGrounded)
    {
        if(playerVelocity.y > -MAX_VELOCITY)
            playerVelocity.y -= GRAVITY * deltaTime * GRAVITY_MULTIPLIER;
    }


}
void Player::UpdatePositionXZ(glm::vec3& newPosition) {

    glm::vec3 newPosX = glm::vec3(newPosition.x, newPosition.y, position.z);
    glm::vec3 newPosZ = glm::vec3(position.x, newPosition.y, newPosition.z);
    if(!isColliding(newPosition, camera.Front))
    {
        if((isShifting && checkNewPositionY(newPosition)) || (isShifting && !isGrounded))
        {
            position.x = newPosition.x;
            position.z = newPosition.z;
        }
        else if((isShifting && checkNewPositionY(newPosX)) || (isShifting && !isGrounded))
        {
            position.x = newPosition.x;
        }
        else if(isShifting && checkNewPositionY(newPosZ) || (isShifting && !isGrounded)){
            position.z = newPosition.z;
        }
        else if(!isShifting)
        {
            position.x = newPosition.x;
            position.z = newPosition.z;
        }

    }
    else{
        glm::vec2 newVelocity;
        if(!checkNewPositionX(newPosition.x) )
        {
            if((isShifting && checkNewPositionY(newPosX)) || (isShifting && !isGrounded))
            {
                position.x = newPosition.x;
            }
            else if(!isShifting)
                position.x = newPosition.x;
            else
                playerVelocity.x = 0;
        }
        else
        {
            playerVelocity.x = 0;
        }
        if(!checkNewPositionZ(newPosition.z))
        {
            if((isShifting && checkNewPositionY(newPosZ)) || (isShifting && !isGrounded))
            {
                position.z = newPosition.z;
            }
            else if(!isShifting)
                position.z = newPosition.z;
            else
                playerVelocity.z = 0;
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
void Player::SortTransparentFaces() {
    Chunk* currentChunk = world->GetChunk(int(glm::round(position.x) / Chunk::SIZE), int(glm::round(position.z) / Chunk::SIZE));

    //when moving inbetween chunks, sort surrounding chunks
    if(chunkPosition.x != currentChunk->chunkPosition.x || chunkPosition.y != currentChunk->chunkPosition.y)
    {
        for(int x = (int)chunkPosition.x - 2; x < (int)chunkPosition.x + 2; x++){
            for(int z = (int)chunkPosition.y - 2; z < (int)chunkPosition.y + 2; z++){
                Chunk* currentChunkToSort = world->GetChunk(x, z);
                if(currentChunkToSort != nullptr && !currentChunkToSort->inThread)
                {
                    world->loadedChunks.push(currentChunkToSort); //loadedchunks sorts each chunk transparent face
                }
            }
        }

    }
    //else sort the current chunk the player is in every time you move a block
    else
    {
        if(currentChunk != nullptr && !currentChunk->inThread && glm::round(lastPosition) != glm::round(position))
            //only sort if block pos has changes hence round
        {
            world->loadedChunks.push(currentChunk); //loadedchunks sorts each chunk transparent face
        }
    }
    if (currentChunk != nullptr)
        chunkPosition = currentChunk->chunkPosition;
}
void Player::ProcessKeyboardMovement(cameraMovement dir, float deltaTime)
{
    glm::vec2 playerVelocityXZ;
    playerVelocityXZ.x = playerVelocity.x;
    playerVelocityXZ.y = playerVelocity.z;

    // Calculate acceleration vector for each direction
    glm::vec3 acceleration = glm::vec3(0.0f);

    if (dir == cameraMovement::FORWARD) {
        acceleration += glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * ACCELERATION;
    }

    if (dir == cameraMovement::BACKWARD) {
        acceleration -= glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z)) * ACCELERATION;
    }

    if (dir == cameraMovement::LEFT) {
        acceleration -= glm::normalize(glm::vec3(camera.Right.x, 0, camera.Right.z)) * ACCELERATION;
    }

    if (dir == cameraMovement::RIGHT) {
        acceleration += glm::normalize(glm:: vec3(camera.Right.x, 0, camera.Right.z)) * ACCELERATION;
    }

// Update playerVelocity based on acceleration
    playerVelocity += acceleration * deltaTime;

// Ensure the total velocity doesn't exceed the maximum speed
    if (glm::length(glm::vec2(playerVelocityXZ)) > movementSpeed) {
        glm::vec2 normalizedXZ = glm::normalize(glm::vec2(playerVelocity.x, playerVelocity.z));
        playerVelocity.x = normalizedXZ.x * movementSpeed;
        playerVelocity.z = normalizedXZ.y * movementSpeed;
    }

    if (dir == cameraMovement::DOWN) {
        isShifting = true;
    }
    if (dir == cameraMovement::UP) {
        if(isGrounded && !isJumping)
        {
            playerVelocity.y = 0;
            isJumping = true;
            playerVelocity.y += 6.20f;
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
    for(int i = -1; i <= 1; i+= 2) {
        float xWidth = i * WIDTH;

        float zWidth = (newZ > position.z) ? WIDTH + 0.01f : -WIDTH - 0.01f;
        glm::ivec2 _chunkPosition;
        _chunkPosition.x = (int)(glm::round(position.x + xWidth) / Chunk::SIZE);
        _chunkPosition.y = (int)(glm::round(newZ + zWidth) / Chunk::SIZE);

        Chunk* currentChunk = world->GetChunk(_chunkPosition.x, _chunkPosition.y);

        int newChunkPosX = (int)glm::round((position.x - (float)currentChunk->chunkPosition.x * Chunk::SIZE) + xWidth);

        int y1 = (int)glm::round(position.y);
        int y2 = (int)glm::round(position.y - 1);
        int y3 = (int)glm::round(position.y - 1.5);

        int newChunkPosZ = (int)glm::round((newZ - (float)currentChunk->chunkPosition.y * Chunk::SIZE) + zWidth);

        if(currentChunk->GetBlockID(glm::vec3(newChunkPosX, y1, newChunkPosZ)) != 0 ||
           currentChunk->GetBlockID(glm::vec3(newChunkPosX, y2, newChunkPosZ)) != 0 ||
           currentChunk->GetBlockID(glm::vec3(newChunkPosX, y3, newChunkPosZ)) != 0)
        {
            return true;
        }
    }
    return false;
}
bool Player::checkNewPositionX(float newX) const
{
    for(int i = -1; i <= 1; i+= 2) {
        float zWidth = i * WIDTH;

        float xWidth = (newX > position.x) ? WIDTH + 0.01f : -WIDTH - 0.01f;
        glm::ivec2 _chunkPosition;
        _chunkPosition.x = (int)(glm::round(newX + xWidth) / Chunk::SIZE);
        _chunkPosition.y = (int)(glm::round(position.z + zWidth) / Chunk::SIZE);

        Chunk* currentChunk = world->GetChunk(_chunkPosition.x, _chunkPosition.y);

        int newChunkPosX = (int)glm::round((newX - (float)currentChunk->chunkPosition.x * Chunk::SIZE) + xWidth);

        int y1 = (int)glm::round(position.y);
        int y2 = (int)glm::round(position.y - 1);
        int y3 = (int)glm::round(position.y - 1.5);

        int newChunkPosZ = (int)glm::round((position.z - (float)currentChunk->chunkPosition.y * Chunk::SIZE) + zWidth);

        if(currentChunk->GetBlockID(glm::vec3(newChunkPosX, y1, newChunkPosZ)) != 0 ||
        currentChunk->GetBlockID(glm::vec3(newChunkPosX, y2, newChunkPosZ)) != 0 ||
        currentChunk->GetBlockID(glm::vec3(newChunkPosX, y3, newChunkPosZ)) != 0)
        {
            return true;
        }
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

    if(currentChunk->GetBlockID(glm::vec3(newChunkPos.x, newChunkPos.y, newChunkPos.z)) ||
            currentChunk->GetBlockID(glm::round(glm::vec3(newChunkPos.x, newChunkPos.y - 1.5f, newChunkPos.z))))
    {
        return true;
    }
    return false;
}
bool Player::checkNewPositionY(glm::vec3& newPosition) const
{
    for(int i = -1; i <= 1; i+= 2)
    {
        for(int j = -1; j <= 1; j+= 2)
        {
            glm::ivec3 _chunkPosition;

            float xWidth = i * WIDTH;
            float zWidth = j * WIDTH;

            _chunkPosition.x = static_cast<int>(std::round(position.x + xWidth) / Chunk::SIZE);
            _chunkPosition.y = static_cast<int>(std::round(position.z + zWidth) / Chunk::SIZE);
            Chunk* currentChunk= world->GetChunk(_chunkPosition.x, _chunkPosition.y);

            glm::vec3 localChunkPos; //position in chunk
            localChunkPos.x = ((newPosition.x - (float)currentChunk->chunkPosition.x * Chunk::SIZE) + xWidth);
            localChunkPos.z = ((newPosition.z - (float)currentChunk->chunkPosition.y * Chunk::SIZE) + zWidth);

            if(currentChunk->GetBlockID(glm::round(glm::vec3(localChunkPos.x, newPosition.y - HEIGHT, localChunkPos.z))) != 0)
            {
                return true;
            }

        }

    }
    return false;
}
float Player::distanceToPlayer(const glm::vec3& point) const {
    //return glm::length(point - position);
    return glm::distance(position, point);
}


