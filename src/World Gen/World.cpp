#include "Chunk.h"
#include "World.h"
#include "Input/Input.h"
#include "ChunkMeshGeneration.h"
#include "Player/Player.h"


World::World(Camera& _camera, Scene& _scene, Player& _player) : camera(_camera), scene(_scene), player(_player)
{
    World::viewDistance = 12;
    playerChunkPos = glm::ivec2((player.position).x / Chunk::SIZE, (player.position).z / Chunk::SIZE); //used for priority queues, chunks closest have priority

    //initialise threads
    chunkThread = std::thread(&World::GenerateChunkThread, this);
    chunkThread.detach();
    worldGenThread = std::thread(&World::GenerateWorldThread, this);
    worldGenThread.detach();

}
int World::viewDistance = 12;
void World::GenerateChunkThread()
{
    volatile bool keepRunning = true;
    while (keepRunning)
    {
        if (!chunksToLoadData.empty())
        {
            Chunk* chunk;
            {
                std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
                chunk = GetChunk(chunksToLoadData.back());
                chunksToLoadData.pop_back();
            }
            if(chunk == nullptr){
                continue;
            }
            if(chunk->chunkDeleteMutex.try_lock()) {
                chunk->inThread = true;

                CheckForBlocksToBeAdded(chunk);
                chunk->LoadChunkData();
                chunk->inThread = false;
                {
                    std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
                    loadedChunks.push(std::ref(chunk->chunkPosition));
                }
                chunk->chunkDeleteMutex.unlock();
            }
        }
    }
}

void World::GenerateWorldThread()
{
    volatile bool keepRunning = true;
    while (keepRunning)
    {
        if (!chunksToGenerate.empty())
        {
            Chunk* chunk;
            {
                std::lock_guard<std::mutex> lock(mutexChunksToGenerate);
                chunk = GetChunk(chunksToGenerate.back());
                chunksToGenerate.pop_back();
            }
            if(chunk == nullptr){

                continue;
            }
            if(chunk->chunkDeleteMutex.try_lock()) {

                chunk->inThread = true;
                chunk->GenBlocks();
                CheckForBlocksToBeAdded(chunk);

                mutexChunksToLoadData.lock();
                chunksToLoadData.insert(chunksToLoadData.begin(), chunk->chunkPosition);
                mutexChunksToLoadData.unlock();
                chunk->chunkDeleteMutex.unlock();
            }
        }
    }
}
bool World::CheckForBlocksToBeAdded(Chunk* chunk)
{
    bool hasBlocksToBeAdded = false;
    mutexBlocksToBeAddedList.lock();

    std::vector<BlocksToBeAdded> newBlocksToBeAddedList;
    for(int i = 0; i < blocksToBeAddedList.size(); i++)
    {
        BlocksToBeAdded &_blocksToBeAdded = blocksToBeAddedList[i];

        if(_blocksToBeAdded.chunkPosition == chunk->chunkPosition)
        {
            //TODO, make id more clear
            chunk->SetBlock(glm::ivec3(_blocksToBeAdded.localPosition.x, _blocksToBeAdded.localPosition.y, _blocksToBeAdded.localPosition.z), 8);
            hasBlocksToBeAdded = true;
        }
        else
        {
            newBlocksToBeAddedList.push_back(_blocksToBeAdded);
        }
    }
    blocksToBeAddedList.clear();
    for(BlocksToBeAdded _blocksToBeAdded : newBlocksToBeAddedList)
    {
        blocksToBeAddedList.push_back(_blocksToBeAdded);
    }

    mutexBlocksToBeAddedList.unlock();
    return hasBlocksToBeAdded;
}
void World::UpdateViewDistance(glm::ivec2& cameraChunkPos)
{
    //update comparator to current chunkPos
    playerChunkPos = cameraChunkPos;
    CompareChunks compareChunks;
    compareChunks._playerChunkPos = cameraChunkPos;

    std::vector<glm::ivec2> generateChunks; //chunks sent to the generation thread, gets sent to other thread to load buffer data
    std::vector<glm::ivec2> addedChunks; //chunks that already have buffer data, no need to send to generate or load
    std::vector<glm::ivec2> newActiveChunks; //chunks that are still in view, no need to process/remove
    std::vector<glm::ivec2> chunksLoading; //chunks that are in cameras view but didn't load yet, let them load, don't remove

    int min_x = (cameraChunkPos.x > viewDistance) ? (cameraChunkPos.x - viewDistance) : 0;
    int min_z = (cameraChunkPos.y > viewDistance) ? (cameraChunkPos.y - viewDistance) : 0;
    int max_x = (SIZE > cameraChunkPos.x + viewDistance) ? (cameraChunkPos.x + viewDistance) : SIZE;
    int max_z = (SIZE > cameraChunkPos.y + viewDistance) ? (cameraChunkPos.y + viewDistance) : SIZE;

    for (int x = min_x; x < max_x; x++)
    {
        for (int z = min_z; z < max_z; z++)
        {

            //if chunk doesn't exist, add it to thread queue for data generation
            if (chunks[x + SIZE * z] == nullptr || (!chunks[x + SIZE * z]->generatedBlockData && !chunks[x + SIZE * z]->inThread))
            {
                chunks[x + SIZE * z] = new Chunk(glm::vec2(x, z), *this);
                generateChunks.push_back(std::ref(chunks[x + SIZE * z]->chunkPosition));
            }
            else if(!chunks[x + SIZE * z]->inThread && chunks[x + SIZE * z]->generatedBuffData)
            {
                newActiveChunks.push_back(std::ref(chunks[x + SIZE * z]->chunkPosition));
            }
            else if(!chunks[x + SIZE * z]->generatedBuffData)
            {
                chunksLoading.push_back(std::ref(chunks[x + SIZE * z]->chunkPosition));
            }

        }

    }
    //All chunks which are not in chunksLoading but in chunksToLoadData to be deleted
    //All chunks which are in activeChunk list but not in new active chunk
    for(glm::ivec2 chunkPos : chunksToLoadData)
    {
        Chunk* chunk = GetChunk(chunkPos);
        if(chunk == nullptr){
            continue;
        }
        if(std::find(chunksLoading.begin(), chunksLoading.end(), chunkPos) == chunksLoading.end() && !chunk->inThread)
        {
            delete chunks[chunkPos.x + SIZE * chunkPos.y];
            chunks[chunkPos.x + SIZE * chunkPos.y] = nullptr;
           // std::cout<<"deleting chunk at: "<<chunkPos.x<<"x "<<chunkPos.y<<"z \n";
        }
    }
    for(glm::ivec2 chunkPos : activeChunks)
    {
        Chunk* chunk = GetChunk(chunkPos);
        if(chunk == nullptr){
            continue;
        }
        if(std::find(newActiveChunks.begin(), newActiveChunks.end(), chunkPos) == newActiveChunks.end() && !chunk->inThread)
        {
            delete chunks[chunkPos.x + SIZE * chunkPos.y];
            chunks[chunkPos.x + SIZE * chunkPos.y] = nullptr;
           // std::cout<<"deleting chunk at: "<<chunkPos.x<<"x "<<chunkPos.y<<"z \n";
        }
    }
    std::sort(generateChunks.begin(), generateChunks.end(), compareChunks);
    std::sort(chunksLoading.begin(), chunksLoading.end(), compareChunks);

    {
        std::lock_guard<std::mutex> lock(mutexChunksToGenerate);
        chunksToGenerate.clear();
        chunksToGenerate = generateChunks;
    }

    {
        std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
        chunksToLoadData.clear();
        chunksToLoadData = chunksLoading;
    }
    activeChunks.clear();
    activeChunks = newActiveChunks;


}


void World::GenerateChunkBuffers(std::vector<Chunk*>& addedChunks)
{
    for (Chunk* chunk : addedChunks)
    {
        if (!chunk->inThread)
        {
            //todo fix this race condition
            //chunk->chunkHasMeshes = false;
            chunk->generatedBuffData = false;
            chunk->LoadBufferData();
            chunk->generatedBuffData = true;
            if(!chunk->chunkHasMeshes)
            {
                mutexChunksToLoadData.lock();
                chunk->chunkHasMeshes = true;
                activeChunks.push_back(chunk->chunkPosition);
                mutexChunksToLoadData.unlock();
            }


        }
    }
    addedChunks.clear();

}
Chunk* World::GetChunk(int x, int y)
{
    if(x >= 0 && x < SIZE && y >= 0 && y < SIZE)
        return chunks[x + SIZE * y];
    else
        return nullptr;
}
Chunk* World::GetChunk(glm::ivec2 pos)
{
    if(pos.x >= 0 && pos.x < SIZE && pos.y >= 0 && pos.y < SIZE)
        return chunks[pos.x + SIZE * pos.y];
    else
        return nullptr;
}
//used for breaking blocks
bool World::RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& currentChunk) {
    float step = 0.0f;
    float reach = 5.0f;

    while (step < reach) {
        glm::ivec3 globalPos;
        globalPos.x = static_cast<int>(std::round(rayOrigin.x + rayDirection.x * step));
        globalPos.y = static_cast<int>(std::round(rayOrigin.y + rayDirection.y * step));
        globalPos.z = static_cast<int>(std::round(rayOrigin.z + rayDirection.z * step));

        glm::ivec2 posInChunks = glm::ivec2(globalPos.x / Chunk::SIZE, globalPos.z / Chunk::SIZE);

        if(posInChunks.x >= 0 && posInChunks.x < World::SIZE && posInChunks.y >= 0 && posInChunks.y < World::SIZE){
            currentChunk = GetChunk(posInChunks.x, posInChunks.y);
            if (currentChunk != nullptr && currentChunk->generatedBlockData) {

                glm::ivec3 localPos;
                localPos.x = globalPos.x - currentChunk->chunkPosition.x * Chunk::SIZE;
                localPos.y = globalPos.y;
                localPos.z = globalPos.z - currentChunk->chunkPosition.y * Chunk::SIZE;
                if (Block::isSolid(currentChunk->GetBlockID(localPos))) {
                    result = localPos;
                    return true;
                }
            }
        }

        step+=0.01f;
    }

    return false;
}
//Used for place blocks
bool World::RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& currentChunk, glm::ivec3& lastEmptyPos) {

    float step = 0.0f;
    float reach = 5.0f;

    while (step < reach) {
        glm::ivec3 globalPos;
        globalPos.x = static_cast<int>(std::round(rayOrigin.x + rayDirection.x * step));
        globalPos.y = static_cast<int>(std::round(rayOrigin.y + rayDirection.y * step));
        globalPos.z = static_cast<int>(std::round(rayOrigin.z + rayDirection.z * step));

        Chunk* tempCurrentChunk = GetChunk(globalPos.x / Chunk::SIZE, globalPos.z / Chunk::SIZE);
        if (tempCurrentChunk!= nullptr && tempCurrentChunk->generatedBlockData) {

            glm::ivec3 localPos;
            localPos.x = globalPos.x - tempCurrentChunk->chunkPosition.x * Chunk::SIZE;
            localPos.y = globalPos.y;
            localPos.z = globalPos.z - tempCurrentChunk->chunkPosition.y * Chunk::SIZE;
            if (Block::isSolid(tempCurrentChunk->GetBlockID(localPos))){
                result = localPos;
                return true;
            }
            else
            {
                lastEmptyPos = localPos;
                currentChunk = GetChunk(globalPos.x / Chunk::SIZE, globalPos.z / Chunk::SIZE);
            }
        }
        step+=0.01f;
    }

    return false;
}
void World::PlaceBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) {
    glm::ivec3 localPos;
    glm::ivec3 lastEmptyPos;
    Chunk* currentChunk;

    // Raycast to find the block to place the face on
    if (RaycastBlockPos(rayOrigin, rayDirection, localPos, currentChunk, lastEmptyPos) &&
        !player.checkCollisionWithBlockLocal(lastEmptyPos)){

        if(currentChunk->generatedBlockData) {
            currentChunk->SetBlock(lastEmptyPos, player.getBlockID());
            std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
            currentChunk->generatedBuffData = false;
            chunksToLoadData.push_back(currentChunk->chunkPosition);
        }
    }
}
void World::BreakBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection){

    glm::ivec3 localPos;
    Chunk* currentChunk;
    if(RaycastBlockPos(rayOrigin, rayDirection, localPos, currentChunk))
    {
        currentChunk->SetBlock(localPos, 0);
        //if block broken was on border, check and update neighbouring chunk mesh
        {
            std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
            currentChunk->generatedBuffData = false;
            chunksToLoadData.push_back(currentChunk->chunkPosition);
        }
        int tempChunkX = currentChunk->chunkPosition.x;
        int tempChunkZ = currentChunk->chunkPosition.y;
        Chunk* tempChunk1 = nullptr;
        Chunk* tempChunk2 = nullptr;
        if(localPos.x == 0 || localPos.x == Chunk::SIZE-1)
        {
            tempChunkX = (localPos.x == 0) ? currentChunk->chunkPosition.x-1 : currentChunk->chunkPosition.x+1;
            tempChunk1 = GetChunk(tempChunkX, currentChunk->chunkPosition.y);
        }
        if(localPos.z == 0 || localPos.z == Chunk::SIZE-1)
        {
            tempChunkZ = (localPos.z == 0) ? currentChunk->chunkPosition.y-1 : currentChunk->chunkPosition.y+1;
            tempChunk2 = GetChunk(currentChunk->chunkPosition.x, tempChunkZ);
        }
        //2 temp chunks just in case we are in corner
        if(tempChunk1 != nullptr && tempChunk1->generatedBlockData)
        {
            std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
            tempChunk1->generatedBuffData = false;
            chunksToLoadData.push_back(tempChunk1->chunkPosition);
        }
        if(tempChunk2 != nullptr && tempChunk2->generatedBlockData)
        {
            std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
            tempChunk2->generatedBuffData = false;
            chunksToLoadData.push_back(tempChunk2->chunkPosition);
        }
    }

}

void World::LoadThreadDataToMain()
{
    if (!loadedChunks.empty())
    {
        std::vector<Chunk*> addedChunks;
        mutexChunksToLoadData.lock();
        for (int i = 0; i < loadedChunks.size(); i++)
        {
            Chunk* chunk = GetChunk(loadedChunks.front());
            if(chunk == nullptr){
                mutexChunksToLoadData.unlock();
                loadedChunks.pop();
                continue;
            }
            if(!chunk->inThread) {
                chunk->sortTransparentMeshData(); //sort transparent faces before rendering
                if (CheckForBlocksToBeAdded(chunk)){
                    loadedChunks.pop();
                    chunksToLoadData.push_back(chunk->chunkPosition);
                }
                else if(!chunk->getIsAllSidesUpdated()){
                    {
                        loadedChunks.pop();
                        std::lock_guard<std::mutex> lock(chunk->chunkMeshMutex);
                        ChunkMeshGeneration::UpdateNeighbours(*chunk);
                        addedChunks.push_back(std::ref(chunk));
                    }
                }
                else{
                    addedChunks.push_back(std::ref(chunk));
                    loadedChunks.pop();
                }

            }
        }
        mutexChunksToLoadData.unlock();
        if (!addedChunks.empty()){
            GenerateChunkBuffers(addedChunks); //adds to active chunks
        }

    }
}
void World::sortChunks(){
    //sort active chunks by farthest from the player in front -> for transparency
    CompareChunks compareChunks;
    compareChunks._playerChunkPos = playerChunkPos;
    std::sort(activeChunks.begin(), activeChunks.end(), compareChunks);
}

void World::sortChunks(glm::vec3 pos){
    CompareChunks compareChunks;
    compareChunks._playerChunkPos = pos;
    std::sort(activeChunks.begin(), activeChunks.end(), compareChunks);
}
void World::sortTransparentFaces() {
    if(player.chunkPosition.x > 0 && player.chunkPosition.x < World::SIZE && player.chunkPosition.y > 0 && player.chunkPosition.y < World::SIZE) {
        Chunk *currentChunk = GetChunk(playerChunkPos.x, playerChunkPos.y);
        if(currentChunk != nullptr) {

            //when moving inbetween chunks, sort surrounding chunks
            if (player.chunkPosition.x != currentChunk->chunkPosition.x ||
                player.chunkPosition.y != currentChunk->chunkPosition.y) {
                for (int x = (int) player.chunkPosition.x - 2;
                     x < (int) player.chunkPosition.x + 2 && x > 0 && x < World::SIZE; x++) {
                    for (int z = (int) player.chunkPosition.y - 2;
                         z < (int) player.chunkPosition.y + 2 && z > 0 && z < World::SIZE; z++) {
                        Chunk *currentChunkToSort = GetChunk(x, z);
                        if (currentChunkToSort != nullptr && !currentChunkToSort->inThread &&
                            currentChunkToSort->generatedBuffData) {
                            loadedChunks.push(currentChunkToSort->chunkPosition); //loadedchunks sorts each chunk transparent face
                        }
                    }
                }

            }
                //else sort the current chunk the player is in every time you move a block
            else {
                if (!currentChunk->inThread &&
                    glm::round(player.lastPosition) != glm::round(player.position) && currentChunk->generatedBuffData)
                    //only sort if block pos has changes hence round
                {
                    loadedChunks.push(currentChunk->chunkPosition); //loadedchunks sorts each chunk transparent face
                }
            }
            player.chunkPosition = currentChunk->chunkPosition;
        }
    }
}
//default way to render
void World::renderChunks()
{
    for (glm::ivec2 chunkPos : activeChunks) {
        Chunk* chunk = GetChunk(chunkPos);

        if (chunk != nullptr) {
            {
                if (chunk->chunkHasMeshes && chunk->mesh != nullptr &&
                chunk->transparentMesh != nullptr && chunk->mesh->loadedData &&
                chunk->transparentMesh->loadedData &&
                !chunk->toBeDeleted) {
                    scene.renderMesh(*chunk->mesh, *scene.shader);
                    glDepthMask(GL_FALSE);
                    scene.renderMesh(*chunk->transparentMesh, *scene.transparentShader);
                    glDepthMask(GL_TRUE);
                }
            }
        }
    }
}
void World::renderTransparentMeshes(Shader& shader) {
    for (glm::ivec2 chunkPos : activeChunks) {
        Chunk* chunk = GetChunk(chunkPos);
        if(chunk != nullptr) {
            if (chunk->chunkHasMeshes && chunk->transparentMesh != nullptr &&
            chunk->transparentMesh->loadedData && !chunk->toBeDeleted) {
                scene.renderMesh(*chunk->transparentMesh, shader);
            }
        }
    }
}
void World::renderChunks(Shader& shader)
{
    for (glm::ivec2 chunkPos : activeChunks) {
        Chunk* chunk = GetChunk(chunkPos);
        if(chunk != nullptr) {
            if (chunk->chunkHasMeshes && chunk->mesh != nullptr && chunk->mesh->loadedData &&
            &shader != nullptr && !chunk->toBeDeleted) {
                scene.renderMesh(*chunk->mesh, shader);
                scene.renderMesh(*chunk->transparentMesh, shader);
            }
        }
    }
}

void World::renderChunks(Shader& shader, glm::vec3 lightPos)
{
    for (glm::ivec2 chunkPos : activeChunks) {
        Chunk* chunk = GetChunk(chunkPos);
        if(chunk != nullptr){
            if (chunk->chunkHasMeshes && chunk->mesh != nullptr && chunk->mesh->loadedData &&
            &shader != nullptr && !chunk->toBeDeleted)
            {
                scene.renderMesh(*chunk->mesh, shader);
                scene.renderMesh(*chunk->transparentMesh, shader);
            }

        }

    }
}
void World::update()
{
    sortTransparentFaces();

    scene.updateShaders();
    //changes global texture every second that passes
    scene.changeGlobalTexture();

    LoadThreadDataToMain();

    sortChunks();
}

void World::renderSolidMeshes(Shader &shader) {
    for (glm::ivec2 chunkPos : activeChunks) {
        Chunk* chunk = GetChunk(chunkPos);
        if(chunk != nullptr) {
                if (chunk->chunkHasMeshes && chunk->mesh != nullptr && chunk->mesh->loadedData && &shader != nullptr &&
                    !chunk->toBeDeleted) {
                    scene.renderMesh(*chunk->mesh, shader);
                }
        }
    }
}
