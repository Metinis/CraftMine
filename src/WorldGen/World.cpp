#include <unordered_set>
#include "Chunk.h"
#include "World.h"
#include "ChunkGeneration.h"
#include "Frustum.h"
#include "Player/Player.h"
#include "Network/NetworkClient.h"
#include "Network/PacketTypes.h"
#include "zlib.h"


World::World(Camera& _camera, SceneRenderer& _scene, Player& _player) : camera(_camera), scene(_scene), player(_player)
{
    loadDataFromFile();
    World::viewDistance = 6;
    playerChunkPos = glm::ivec2((player.position).x / Chunk::SIZE, (player.position).z / Chunk::SIZE); //used for priority queues, chunks closest have priority

    WorldThreading::startThreads(*this);
}
int World::viewDistance = 6;


void World::UpdateViewDistance(const glm::ivec2& cameraChunkPos)
{
    //update comparator to current chunkPos
    playerChunkPos = cameraChunkPos;
    CompareChunks compareChunks;
    compareChunks._playerChunkPos = cameraChunkPos;

    std::vector<glm::ivec2> generateChunks; //chunks sent to the generation thread, gets sent to other thread to load buffer data
    std::vector<glm::ivec2> addedChunks; //chunks that already have buffer data, no need to send to generate or load
    std::vector<glm::ivec2> newActiveChunks; //chunks that are still in view, no need to process/remove
    std::vector<glm::ivec2> chunksLoading; //chunks that are in cameras view but didn't load yet, let them load, don't remove

    const int min_x = (cameraChunkPos.x > viewDistance) ? (cameraChunkPos.x - viewDistance) : 0;
    const int min_z = (cameraChunkPos.y > viewDistance) ? (cameraChunkPos.y - viewDistance) : 0;
    const int max_x = (SIZE > cameraChunkPos.x + viewDistance) ? (cameraChunkPos.x + viewDistance) : SIZE;
    const int max_z = (SIZE > cameraChunkPos.y + viewDistance) ? (cameraChunkPos.y + viewDistance) : SIZE;

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

    //delete inactive chunks from threads
    WorldThreading::deleteInActiveChunks(*this, chunksLoading, newActiveChunks, activeChunks);

    //Sort chunks
    std::sort(generateChunks.begin(), generateChunks.end(), compareChunks);
    std::sort(chunksLoading.begin(), chunksLoading.end(), compareChunks);

    if (multiplayerMode && networkClient != nullptr) {
        for (size_t i = 0; i < generateChunks.size(); i++) {
            const glm::ivec2& pos = generateChunks[i];
            std::vector<uint8_t> payload = PacketSerializer::serializeRequestChunk(pos.x, pos.y);
            networkClient->sendPacket(PacketType::C2S_REQUEST_CHUNK, payload);
        }
    } else {
        WorldThreading::setChunksToGenerate(generateChunks);
    }
    WorldThreading::setChunksToLoadData(chunksLoading);

    activeChunks.clear();
    activeChunks = newActiveChunks;

    saveBlocksToBeAddedToFile();
}
void World::deleteChunk(const glm::ivec2 pos) {
    delete chunks[pos.x + SIZE * pos.y];
    chunks[pos.x + SIZE * pos.y] = nullptr;
}

void World::receiveServerChunk(int cx, int cz, const std::vector<uint8_t>& compressedData) {
    if (cx < 0 || cx >= SIZE || cz < 0 || cz >= SIZE) {
        std::cerr << "[World] Received chunk at invalid position: " << cx << ", " << cz << std::endl;
        return;
    }

    Chunk* chunk = chunks[cx + SIZE * cz];
    if (chunk == nullptr) {
        chunk = new Chunk(glm::ivec2(cx, cz), *this);
        chunks[cx + SIZE * cz] = chunk;
    }

    uLongf decompressedSize = sizeof(chunk->blockIDs);
    int result = uncompress(chunk->blockIDs, &decompressedSize,
                            compressedData.data(), static_cast<uLong>(compressedData.size()));
    if (result != Z_OK) {
        std::cerr << "[World] Failed to decompress chunk data for (" << cx << ", " << cz << ")" << std::endl;
        return;
    }

    chunk->generatedBlockData = true;

    WorldThreading::updateLoadData(chunk);

    std::cout << "[World] Received server chunk (" << cx << ", " << cz << ")" << std::endl;
}

void World::GenerateChunkBuffers(std::vector<Chunk*>& addedChunks)
{
    for (Chunk* chunk : addedChunks)
    {
            chunk->LoadBufferData();
            {
                chunk->chunkHasMeshes = true;
                if(std::find(activeChunks.begin(), activeChunks.end(), chunk->chunkPosition) == activeChunks.end())
                    activeChunks.push_back(chunk->chunkPosition);
            }
    }
    addedChunks.clear();

}
Chunk* World::GetChunk(const int x, const int y) const
{
    if(x >= 0 && x < SIZE && y >= 0 && y < SIZE)
        return chunks[x + SIZE * y];
    else
        return nullptr;
}
Chunk* World::GetChunk(const glm::ivec2 pos) const
{
    if(pos.x >= 0 && pos.x < SIZE && pos.y >= 0 && pos.y < SIZE)
        return chunks[pos.x + SIZE * pos.y];
    else
        return nullptr;
}
//used for breaking blocks
bool World::RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& currentChunk) const {
    float step = 0.0f;
    constexpr float reach = 5.0f;

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
                if (Block::hasCustomMesh(currentChunk->GetBlockID(localPos))) {
                    if(std::abs(static_cast<float>(roundedGlobalPos.x) - globalPos.x) < 0.2f &&
                        std::abs(static_cast<float>(roundedGlobalPos.y) - globalPos.y) < 0.3f &&
                        std::abs(static_cast<float>(roundedGlobalPos.z) - globalPos.z) < 0.3f
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
bool World::RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& currentChunk, glm::ivec3& lastEmptyPos) const {

    float step = 0.0f;
    constexpr float reach = 5.0f;

    while (step < reach) {
        glm::ivec3 globalPos;
        globalPos.x = static_cast<int>(std::round(rayOrigin.x + rayDirection.x * step));
        globalPos.y = static_cast<int>(std::round(rayOrigin.y + rayDirection.y * step));
        globalPos.z = static_cast<int>(std::round(rayOrigin.z + rayDirection.z * step));

        const Chunk* tempCurrentChunk = GetChunk(globalPos.x / Chunk::SIZE, globalPos.z / Chunk::SIZE);
        if (tempCurrentChunk!= nullptr && tempCurrentChunk->generatedBlockData) {

            glm::ivec3 localPos;
            localPos.x = globalPos.x - tempCurrentChunk->chunkPosition.x * Chunk::SIZE;
            localPos.y = globalPos.y;
            localPos.z = globalPos.z - tempCurrentChunk->chunkPosition.y * Chunk::SIZE;
            const unsigned char currentBlockID = tempCurrentChunk->GetBlockID(localPos);
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
void World::PlaceBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const{
    glm::ivec3 localPos;
    glm::ivec3 lastEmptyPos;
    Chunk* currentChunk;

    // Raycast to find the block to place the face on
    if (RaycastBlockPos(rayOrigin, rayDirection, localPos, currentChunk, lastEmptyPos) &&
        !player.checkCollisionWithBlockLocal(lastEmptyPos)){

        if(currentChunk->generatedBlockData) {
            currentChunk->SetBlock(lastEmptyPos, player.getBlockID());
            //ChunkLighting::addLightingValues(*currentChunk);
            if(player.getBlockID() == 5) {
                ChunkGeneration::UpdateWater(*currentChunk, lastEmptyPos);
            }
            WorldThreading::updateLoadData(currentChunk);
        }
    }
}
void World::BreakBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const{

    glm::ivec3 localPos;
    Chunk* currentChunk;
    if(RaycastBlockPos(rayOrigin, rayDirection, localPos, currentChunk))
    {
        if(currentChunk->GetBlockID(localPos) == 45) {
            currentChunk->SetBlock(localPos, 0);
            const auto worldPos = glm::ivec3(localPos.x + currentChunk->chunkPosition.x * Chunk::SIZE, localPos.y, localPos.z + currentChunk->chunkPosition.y * Chunk::SIZE);
            //ChunkLighting::clearLight(*currentChunk, worldPos);
            //ChunkLighting::recalculateLightWithNeighbours(*currentChunk);
        }
        else {
            currentChunk->SetBlock(localPos, 0);
        }


        //ChunkLighting::recalculateLightWithNeighbours(*currentChunk);



        WorldThreading::updateLoadData(currentChunk);
        Chunk* tempChunk1 = nullptr;
        Chunk* tempChunk2 = nullptr;
        if(localPos.x == 0 || localPos.x == Chunk::SIZE-1)
        {
            const int tempChunkX = (localPos.x == 0) ? currentChunk->chunkPosition.x-1 : currentChunk->chunkPosition.x+1;
            tempChunk1 = GetChunk(tempChunkX, currentChunk->chunkPosition.y);
        }
        if(localPos.z == 0 || localPos.z == Chunk::SIZE-1)
        {
            const int tempChunkZ = (localPos.z == 0) ? currentChunk->chunkPosition.y-1 : currentChunk->chunkPosition.y+1;
            tempChunk2 = GetChunk(currentChunk->chunkPosition.x, tempChunkZ);
        }
        if(tempChunk1 != nullptr && tempChunk1->generatedBlockData)
        {
            WorldThreading::updateLoadData(tempChunk1);
        }
        if(tempChunk2 != nullptr && tempChunk2->generatedBlockData)
        {
            WorldThreading::updateLoadData(tempChunk2);
        }
    }

}

void World::LoadThreadDataToMain()
{
    std::vector<Chunk*> addedChunks = WorldThreading::GetAddedThreadChunks(*this);

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

void World::sortChunks(const glm::vec3 pos){
    CompareChunks compareChunks;
    compareChunks._playerChunkPos = pos;
    std::sort(activeChunks.begin(), activeChunks.end(), compareChunks);
}
void World::sortTransparentFaces() const{
    if(player.chunkPosition.x > 0 && player.chunkPosition.x < World::SIZE && player.chunkPosition.y > 0 && player.chunkPosition.y < World::SIZE) {
        const Chunk *currentChunk = GetChunk(static_cast<int>(playerChunkPos.x),static_cast<int>(playerChunkPos.y));
        if(currentChunk != nullptr) {

            //when moving inbetween chunks, sort surrounding chunks
            if (player.chunkPosition.x != currentChunk->chunkPosition.x ||
                player.chunkPosition.y != currentChunk->chunkPosition.y) {
                for (int x = (int) player.chunkPosition.x - 2;
                     x < (int) player.chunkPosition.x + 2 && x > 0 && x < World::SIZE; x++) {
                    for (int z = (int) player.chunkPosition.y - 2;
                         z < (int) player.chunkPosition.y + 2 && z > 0 && z < World::SIZE; z++) {
                        const Chunk *currentChunkToSort = GetChunk(x, z);
                        if (currentChunkToSort != nullptr && !currentChunkToSort->inThread &&
                            currentChunkToSort->generatedBuffData) {

                            WorldThreading::addToLoadedChunks(currentChunkToSort);
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
                    WorldThreading::addToLoadedChunks(currentChunk);
                }
            }
            player.chunkPosition = currentChunk->chunkPosition;
        }
    }
}
//default way to render
void World::renderChunksToNormalShaders() const
{
    for (const glm::ivec2 &chunkPos : activeChunks) {
        const Chunk* chunk = GetChunk(chunkPos);

        if (chunk != nullptr) {
            {
                if (chunk->chunkHasMeshes && chunk->mesh != nullptr &&
                chunk->transparentMesh != nullptr && chunk->mesh->loadedData &&
                chunk->transparentMesh->loadedData &&
                !chunk->toBeDeleted) {
                    if(Frustum::isChunkInFrustum(chunk->getChunkMinBounds(), chunk->getChunkMaxBounds(), frustum)){

                        SceneRenderer::renderMesh(*chunk->transparentMesh, *scene.transparentShader);
                        SceneRenderer::renderMesh(*chunk->mesh, *scene.shader);
                    }
                }
            }
        }
    }
}
void World::renderTransparentMeshes(Shader& shader) const{
    for (const glm::ivec2 &chunkPos : activeChunks) {
        const Chunk* chunk = GetChunk(chunkPos);
        if(chunk != nullptr) {
            if (chunk->chunkHasMeshes && chunk->transparentMesh != nullptr &&
            chunk->transparentMesh->loadedData && !chunk->toBeDeleted) {

                if(Frustum::isChunkInFrustum(chunk->getChunkMinBounds(), chunk->getChunkMaxBounds(), frustum))
                    SceneRenderer::renderMesh(*chunk->transparentMesh, shader);
            }
        }
    }
}
void World::renderChunksToShader(Shader& shader) const
{
    for (const glm::ivec2 &chunkPos : activeChunks) {
        const Chunk* chunk = GetChunk(chunkPos);
        if(chunk != nullptr) {
            if (chunk->chunkHasMeshes && chunk->mesh != nullptr && chunk->mesh->loadedData &&
            !chunk->toBeDeleted) {
                if(Frustum::isChunkInFrustum(chunk->getChunkMinBounds(), chunk->getChunkMaxBounds(), frustum)) {



                    SceneRenderer::renderMesh(*chunk->transparentMesh, shader);
                    SceneRenderer::renderMesh(*chunk->mesh, shader);
                }

            }
        }
    }
}
void World::renderChunksToShadow(Shader& shader) const {
    for (const glm::ivec2 &chunkPos : activeChunks) {
        const Chunk* chunk = GetChunk(chunkPos);
        if(chunk != nullptr) {
            if (chunk->chunkHasMeshes && chunk->mesh != nullptr && chunk->mesh->loadedData &&
            !chunk->toBeDeleted) {

                if(Frustum::isChunkInFrustum(chunk->getChunkMinBounds(), chunk->getChunkMaxBounds(), frustum)) {

                    SceneRenderer::renderMesh(*chunk->mesh, shader);

                    SceneRenderer::renderMesh(*chunk->transparentMesh, shader);

                }
            }
        }
    }
}

void World::update()
{
    frustum = Frustum::createFrustumFromCamera(camera, (float)(16.0f/9.0f), 65.0f, 0.1f, 1000.0f);

    sortTransparentFaces();

    scene.updateShaders();
    //changes global texture every second that passes
    scene.changeGlobalTexture();

    LoadThreadDataToMain();

    sortChunks();
}

void World::updateTick() {


    if(!liquidToBeChecked.empty()) {
        std::vector<BlocksToBeAdded> thisLiquidToBeChecked;
        std::vector<glm::ivec2> updatedChunks;

        thisLiquidToBeChecked.reserve(liquidToBeChecked.size());

        for(BlocksToBeAdded block : liquidToBeChecked){

            thisLiquidToBeChecked.push_back(block);
        }
        liquidToBeChecked.clear();

        for(const BlocksToBeAdded block : thisLiquidToBeChecked) {
            Chunk* currentChunk = GetChunk(block.chunkPosition);
            if(currentChunk == nullptr) {
                continue;
            }
            if(currentChunk->generatedBlockData) {

                currentChunk->SetBlock(block.localPosition, block.blockID);
                ChunkGeneration::UpdateWater(*currentChunk, block.localPosition);
                if(std::find(updatedChunks.begin(), updatedChunks.end(), currentChunk->chunkPosition) == updatedChunks.end()) {
                    updatedChunks.push_back(currentChunk->chunkPosition);
                }
            }
        }
        for(const glm::ivec2 chunkPos : updatedChunks) {
            Chunk* currentChunk = GetChunk(chunkPos);
            WorldThreading::updateLoadData(currentChunk);
        }
    }
}

void World::renderSolidMeshes(Shader &shader) const{
    for (const glm::ivec2 chunkPos : activeChunks) {
        const Chunk* chunk = GetChunk(chunkPos);
        if(chunk != nullptr) {
                if (chunk->chunkHasMeshes && chunk->mesh != nullptr && chunk->mesh->loadedData && 
                    !chunk->toBeDeleted) {

                    if(Frustum::isChunkInFrustum(chunk->getChunkMinBounds(), chunk->getChunkMaxBounds(), frustum))
                        SceneRenderer::renderMesh(*chunk->mesh, shader);
                }
        }
    }
}
void World::loadDataFromFile() {

    std::lock_guard<std::mutex> lock(WorldThreading::mutexBlocksToBeAddedList);
    const std::string filename = "../save/blocksToBeAdded.bin";
    std::ifstream infile(filename, std::ios::binary | std::ios::ate);
    if (!infile) {
        return;
    }

    const std::streamsize dataSize = infile.tellg();
    infile.seekg(0, std::ios::beg);

    auto* serializedData = new unsigned char[dataSize];
    if (!infile.read(reinterpret_cast<char*>(serializedData), dataSize)) {
        delete[] serializedData;
        return;
    }
    infile.close();

    // Deserialize the data
    const size_t numBlocks = dataSize / sizeof(BlocksToBeAdded);
    WorldThreading::blocksToBeAddedList.resize(numBlocks);
    memcpy(WorldThreading::blocksToBeAddedList.data(), serializedData, dataSize);

    delete[] serializedData;
    std::cout << "Data successfully loaded from file" << std::endl;
}

void World::saveBlocksToBeAddedToFile() {
    std::lock_guard<std::mutex> lock(WorldThreading::mutexBlocksToBeAddedList);

    // Calculate the size of the serialized data
    const size_t dataSize = WorldThreading::blocksToBeAddedList.size() * sizeof(BlocksToBeAdded);

    // Serialize the data
    auto* serializedData = new unsigned char[dataSize];
    memcpy(serializedData, WorldThreading::blocksToBeAddedList.data(), dataSize);

    // Write the serialized data to a file
    const std::string filename = "../save/blocksToBeAdded.bin";
    std::ofstream outfile(filename, std::ios::binary | std::ios::trunc);
    if (!outfile) {
        delete[] serializedData;
        return;
    }

    outfile.write(reinterpret_cast<const char*>(serializedData), static_cast<long>(dataSize));
    outfile.close();

    delete[] serializedData;
}



