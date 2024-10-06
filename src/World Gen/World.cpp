#include <unordered_set>
#include "Chunk.h"
#include "World.h"
#include "Input/Input.h"
#include "ChunkMeshGeneration.h"
#include "Player/Player.h"


World::World(Camera& _camera, Scene& _scene, Player& _player) : camera(_camera), scene(_scene), player(_player)
{
    loadDataFromFile();
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
        std::unique_lock<std::mutex> lock(mutexChunksToLoadData);
        if (!chunksToLoadData.empty())
        {
            Chunk* chunk;
            {
                chunk = GetChunk(chunksToLoadData.back());
            }
            if(chunk == nullptr){

                chunksToLoadData.pop_back();

                lock.unlock();
                continue;
            }
            if(chunk->chunkDeleteMutex.try_lock()) {

                chunksToLoadData.pop_back();

                lock.unlock();

                chunk->inThread = true;

                CheckForBlocksToBeAdded(chunk);
                chunk->LoadChunkData();
                chunk->inThread = false;
                {
                    std::lock_guard<std::mutex> _lock(mutexLoadedChunks);
                    loadedChunks.push(chunk->chunkPosition);
                }
                chunk->chunkDeleteMutex.unlock();
            }
            else{
                lock.unlock();
            }
        }
        else{
          lock.unlock();
        }
    }
}

void World::GenerateWorldThread()
{
    volatile bool keepRunning = true;
    while (keepRunning)
    {
        std::unique_lock<std::mutex> lock(mutexChunksToGenerate);
        if (!chunksToGenerate.empty())
        {
            Chunk* chunk;
            {
                chunk = GetChunk(chunksToGenerate.back());
            }
            if(chunk == nullptr){

                chunksToGenerate.pop_back();
                lock.unlock();
                continue;
            }
            if(chunk->chunkDeleteMutex.try_lock()) {

                chunksToGenerate.pop_back();
                lock.unlock();
                chunk->inThread = true;
                chunk->GenBlocks();
                CheckForBlocksToBeAdded(chunk);

                {
                    std::lock_guard<std::mutex> _lock(mutexChunksToLoadData);

                    chunksToLoadData.insert(chunksToLoadData.begin(), chunk->chunkPosition);
                }

                chunk->chunkDeleteMutex.unlock();
            }
            else{
                lock.unlock();
            }
        }
        else{
          lock.unlock();
        }
    }
}
bool World::CheckForBlocksToBeAdded(Chunk* chunk)
{

    std::lock_guard<std::mutex> lock(mutexBlocksToBeAddedList);
    bool hasBlocksToBeAdded = false;
    std::vector<BlocksToBeAdded> newBlocksToBeAddedList;
    for(int i = 0; i < blocksToBeAddedList.size(); i++)
    {
        BlocksToBeAdded &_blocksToBeAdded = blocksToBeAddedList[i];

        if(_blocksToBeAdded.chunkPosition == chunk->chunkPosition)
        {
            //TODO, make id more clear
            chunk->SetBlock(glm::ivec3(_blocksToBeAdded.localPosition.x, _blocksToBeAdded.localPosition.y, _blocksToBeAdded.localPosition.z), _blocksToBeAdded.blockID);
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
                generateChunks.push_back((chunks[x + SIZE * z]->chunkPosition));
            }
            else if((chunks[x + SIZE * z]->generatedBuffData && !chunks[x + SIZE * z]->inThread) ||
            (std::find(activeChunks.begin(), activeChunks.end(), chunks[x + SIZE * z]->chunkPosition) != activeChunks.end()) )
            {
                newActiveChunks.push_back((chunks[x + SIZE * z]->chunkPosition));
            }
            else
            {
                chunksLoading.push_back((chunks[x + SIZE * z]->chunkPosition));
            }

        }

    }
    //All chunks which are not in chunksLoading but in chunksToLoadData to be deleted
    //All chunks which are in activeChunk list but not in new active chunk
    std::unordered_set<glm::ivec2, ChunkPosHash> chunksToKeep(chunksLoading.begin(), chunksLoading.end());
    chunksToKeep.insert(newActiveChunks.begin(), newActiveChunks.end());


    {
        std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
        for (glm::ivec2 chunkPos : chunksToLoadData) {
            Chunk* chunk = GetChunk(chunkPos);
            if (chunk == nullptr) {
                continue;
            }
            if (chunksToKeep.find(chunkPos) == chunksToKeep.end() ) {
                delete chunks[chunkPos.x + SIZE * chunkPos.y];
                chunks[chunkPos.x + SIZE * chunkPos.y] = nullptr;
            }
        }
    }

        for (glm::ivec2 chunkPos : activeChunks) {
            Chunk* chunk = GetChunk(chunkPos);
            if (chunk == nullptr) {
                continue;
            }
            if (chunksToKeep.find(chunkPos) == chunksToKeep.end()) {
                delete chunks[chunkPos.x + SIZE * chunkPos.y];
                chunks[chunkPos.x + SIZE * chunkPos.y] = nullptr;
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

    saveBlocksToBeAddedToFile();
    //player.savePosToFile();
}


void World::GenerateChunkBuffers(std::vector<Chunk*>& addedChunks)
{
    for (Chunk* chunk : addedChunks)
    {
            chunk->LoadBufferData();
            {
                //std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
                chunk->chunkHasMeshes = true;
                if(std::find(activeChunks.begin(), activeChunks.end(), chunk->chunkPosition) == activeChunks.end())
                    activeChunks.push_back(chunk->chunkPosition);
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
        glm::vec3 globalPos;
        globalPos.x = rayOrigin.x + rayDirection.x * step;
        globalPos.y = rayOrigin.y + rayDirection.y * step;
        globalPos.z = rayOrigin.z + rayDirection.z * step;

        const glm::ivec3 roundedGlobalPos= glm::round(globalPos);

        const auto posInChunks = glm::ivec2(roundedGlobalPos.x / Chunk::SIZE, roundedGlobalPos.z / Chunk::SIZE);

        if(posInChunks.x >= 0 && posInChunks.x < World::SIZE && posInChunks.y >= 0 && posInChunks.y < World::SIZE){
            currentChunk = GetChunk(posInChunks.x, posInChunks.y);
            if (currentChunk != nullptr && currentChunk->generatedBlockData) {

                glm::ivec3 localPos;
                localPos.x = roundedGlobalPos.x - currentChunk->chunkPosition.x * Chunk::SIZE;
                localPos.y = roundedGlobalPos.y;
                localPos.z = roundedGlobalPos.z - currentChunk->chunkPosition.y * Chunk::SIZE;
                if (Block::isSolid(currentChunk->GetBlockID(localPos))) {
                    result = localPos;
                    return true;
                }
                else if (Block::hasCustomMesh(currentChunk->GetBlockID(localPos))) {
                    if(std::abs(roundedGlobalPos.x - globalPos.x) < 0.2f &&
                        std::abs(roundedGlobalPos.y - globalPos.y) < 0.3f &&
                        std::abs(roundedGlobalPos.z - globalPos.z) < 0.3f
                    ) {
                        result = localPos;
                        return true;
                    }
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
            unsigned char currentBlockID = tempCurrentChunk->GetBlockID(localPos);
            if (Block::isSolid(currentBlockID)){
                result = localPos;
                return true;
            }
            else if(!Block::hasCustomMesh(currentBlockID))
            {
                lastEmptyPos = localPos;
                currentChunk = GetChunk(globalPos.x / Chunk::SIZE, globalPos.z / Chunk::SIZE);
            }
            else {
                return true;
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
            //std::lock_guard<std::mutex> _lock(currentChunk->chunkMeshMutex);
            if(std::find(chunksToLoadData.begin(), chunksToLoadData.end(), currentChunk->chunkPosition) == chunksToLoadData.end()) {
                currentChunk->generatedBuffData = false;
                chunksToLoadData.push_back(currentChunk->chunkPosition);
            }
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
            //std::lock_guard<std::mutex> _lock(tempChunk1->chunkMeshMutex);

            if(std::find(chunksToLoadData.begin(), chunksToLoadData.end(), tempChunk1->chunkPosition) == chunksToLoadData.end()) {
                tempChunk1->generatedBuffData = false;
                chunksToLoadData.push_back(tempChunk1->chunkPosition);
            }
        }
        if(tempChunk2 != nullptr && tempChunk2->generatedBlockData)
        {
            std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
            //std::lock_guard<std::mutex> _lock(tempChunk2->chunkMeshMutex);

            if(std::find(chunksToLoadData.begin(), chunksToLoadData.end(), tempChunk2->chunkPosition) == chunksToLoadData.end()) {
                tempChunk2->generatedBuffData = false;
                chunksToLoadData.push_back(tempChunk2->chunkPosition);
            }
        }
    }

}

void World::LoadThreadDataToMain()
{
        std::vector<Chunk*> addedChunks;
        while(!loadedChunks.empty())
        {
            Chunk* chunk;
            {
                std::lock_guard<std::mutex> lock(mutexLoadedChunks);
                chunk = GetChunk(loadedChunks.front());

                loadedChunks.pop();
            }
            if(chunk == nullptr){
                continue;
            }
            if(!chunk->inThread) {
                chunk->sortTransparentMeshData(); //sort transparent faces before rendering
                if (CheckForBlocksToBeAdded(chunk)){
                    {
                        std::lock_guard<std::mutex> _lock(mutexChunksToLoadData);
                        chunksToLoadData.push_back(chunk->chunkPosition);
                    }
                }
                else if(!chunk->getIsAllSidesUpdated()){
                    {
                        ChunkMeshGeneration::UpdateNeighbours(*chunk);
                        addedChunks.push_back(chunk);
                    }
                }
                else{
                    addedChunks.push_back(chunk);
                }

            }
        }
    if (!addedChunks.empty()){
        GenerateChunkBuffers(addedChunks); //adds to active chunks
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

                            //wstd::lock_guard<std::mutex> lock(mutexLoadedChunks);
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
                    //std::lock_guard<std::mutex> lock(mutexLoadedChunks);
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
            !chunk->toBeDeleted) {
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
            !chunk->toBeDeleted)
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
                if (chunk->chunkHasMeshes && chunk->mesh != nullptr && chunk->mesh->loadedData && 
                    !chunk->toBeDeleted) {
                    scene.renderMesh(*chunk->mesh, shader);
                }
        }
    }
}
void World::loadDataFromFile() {

    std::lock_guard<std::mutex> lock(mutexBlocksToBeAddedList);
    //if(mutexBlocksToBeAddedList.try_lock()){
    std::string filename = "../save/blocksToBeAdded.bin";
    std::ifstream infile(filename, std::ios::binary | std::ios::ate);
    if (!infile) {
        //std::cerr << "Failed to open file for reading: " << filename << std::endl;
        return;
    }

    std::streamsize dataSize = infile.tellg();
    infile.seekg(0, std::ios::beg);

    unsigned char* serializedData = new unsigned char[dataSize];
    if (!infile.read(reinterpret_cast<char*>(serializedData), dataSize)) {
        //std::cerr << "Failed to read data" << std::endl;
        delete[] serializedData;
        return;
    }
    infile.close();

    // Deserialize the data
    size_t numBlocks = dataSize / sizeof(BlocksToBeAdded);
    blocksToBeAddedList.resize(numBlocks);
    memcpy(blocksToBeAddedList.data(), serializedData, dataSize);

    delete[] serializedData;
    std::cout << "Data successfully loaded from file" << std::endl;

    //mutexBlocksToBeAddedList.unlock();
    //}
}

void World::saveBlocksToBeAddedToFile() {
    std::lock_guard<std::mutex> lock(mutexBlocksToBeAddedList);

    //if(mutexBlocksToBeAddedList.try_lock()){

    // Calculate the size of the serialized data
    size_t dataSize = blocksToBeAddedList.size() * sizeof(BlocksToBeAdded);

    // Serialize the data
    unsigned char* serializedData = new unsigned char[dataSize];
    memcpy(serializedData, blocksToBeAddedList.data(), dataSize);

    // Write the serialized data to a file
    std::string filename = "../save/blocksToBeAdded.bin";
    std::ofstream outfile(filename, std::ios::binary | std::ios::trunc);
    if (!outfile) {
        //std::cerr << "Failed to open file for writing: " << filename << std::endl;
        delete[] serializedData;
        return;
    }

    outfile.write(reinterpret_cast<const char*>(serializedData), dataSize);
    outfile.close();

    delete[] serializedData;
    //    mutexBlocksToBeAddedList.unlock();
    //}
}

