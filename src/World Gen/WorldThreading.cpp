#include "WorldThreading.h"

#include "ChunkLighting.h"


class Chunk;

std::thread WorldThreading::chunkThread;
std::thread WorldThreading::worldGenThread;

std::vector<glm::ivec2> WorldThreading::chunksToGenerate;  //used for world gen for chunks that haven't been generated yet -> generates blocks
std::vector<glm::ivec2> WorldThreading::chunksToLoadData; //used to load faces
std::vector<glm::ivec2> WorldThreading::chunksToDelete;
std::vector<BlocksToBeAdded> WorldThreading::blocksToBeAddedList;
std::queue<glm::ivec2> WorldThreading::loadedChunks;

std::mutex WorldThreading::mutexChunksToLoadData;
std::mutex WorldThreading::mutexLoadedChunks;
std::mutex WorldThreading::mutexLiquidBlocks;
std::mutex WorldThreading::mutexChunksToGenerate;
std::mutex WorldThreading::mutexBlocksToBeAddedList;


void WorldThreading::startThreads(World& world) {
    chunkThread = std::thread(&WorldThreading::GenerateChunkThread, std::ref(world));
    chunkThread.detach();
    worldGenThread = std::thread(&WorldThreading::GenerateWorldThread, std::ref(world));
    worldGenThread.detach();
}


void WorldThreading::GenerateChunkThread(const World& world)
{
    constexpr volatile bool keepRunning = true;
    while (keepRunning)
    {
        std::unique_lock<std::mutex> lock(mutexChunksToLoadData);
        if (!chunksToLoadData.empty())
        {
            Chunk* chunk;
            {
                chunk = world.GetChunk(chunksToLoadData.back());
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
                //ChunkLighting::addLightingValues(*chunk);
                //ChunkLighting::recalculateLightWithNeighbours(*chunk);
                //chunk->genLight();
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

void WorldThreading::GenerateWorldThread(const World& world)
{
    constexpr volatile bool keepRunning = true;
    while (keepRunning)
    {
        std::unique_lock<std::mutex> lock(mutexChunksToGenerate);
        if (!chunksToGenerate.empty())
        {
            Chunk* chunk;
            {
                chunk = world.GetChunk(chunksToGenerate.back());
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
                chunk->genBlocks();
                ChunkLighting::initialiseLight(*chunk);
                //ChunkLighting::recalculateLightWithNeighbours(*chunk);
                chunk->genLight();

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
bool WorldThreading::CheckForBlocksToBeAdded(Chunk* chunk)
{
    std::lock_guard<std::mutex> lock(mutexBlocksToBeAddedList);
    bool hasBlocksToBeAdded = false;
    std::vector<BlocksToBeAdded> newBlocksToBeAddedList;
    for(auto & _blocksToBeAdded : blocksToBeAddedList)
    {
        if(_blocksToBeAdded.chunkPosition == chunk->chunkPosition)
        {
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
void WorldThreading::deleteInActiveChunks(World& world, const std::vector<glm::ivec2>& chunksLoading, const std::vector<glm::ivec2>& newActiveChunks, const std::vector<glm::ivec2>& activeChunks) {
    //All chunks which are not in chunksLoading but in chunksToLoadData to be deleted
    //All chunks which are in activeChunk list but not in new active chunk
    std::unordered_set<glm::ivec2, ChunkPosHash> chunksToKeep(chunksLoading.begin(), chunksLoading.end());
    chunksToKeep.insert(newActiveChunks.begin(), newActiveChunks.end());
    {
        std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
        for (glm::ivec2 chunkPos : chunksToLoadData) {
            const Chunk* chunk = world.GetChunk(chunkPos);
            if (chunk == nullptr) {
                continue;
            }
            if (chunksToKeep.find(chunkPos) == chunksToKeep.end() ) {
                world.deleteChunk(chunkPos);
            }
        }
    }
    for (glm::ivec2 chunkPos : activeChunks) {
        const Chunk* chunk = world.GetChunk(chunkPos);
        if (chunk == nullptr) {
            continue;
        }
        if (chunksToKeep.find(chunkPos) == chunksToKeep.end()) {
            world.deleteChunk(chunkPos);
        }
    }
}
void WorldThreading::setChunksToGenerate(const std::vector<glm::ivec2>& generateChunks) {
    std::lock_guard<std::mutex> lock(mutexChunksToGenerate);
    chunksToGenerate.clear();
    chunksToGenerate = generateChunks;
}
void WorldThreading::setChunksToLoadData(const std::vector<glm::ivec2>& chunksLoading) {
    std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
    chunksToLoadData.clear();
    chunksToLoadData = chunksLoading;
}
void WorldThreading::updateLoadData(Chunk* currentChunk) {
    std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
    if(std::find(chunksToLoadData.begin(), chunksToLoadData.end(), currentChunk->chunkPosition) == chunksToLoadData.end()) {
        currentChunk->generatedBuffData = false;
        chunksToLoadData.push_back(currentChunk->chunkPosition);
    }
}
std::vector<Chunk*> WorldThreading::GetAddedThreadChunks(const World& world)
{
    std::vector<Chunk*> addedChunks;
    while(!loadedChunks.empty())
    {
        Chunk* chunk;
        {
            std::lock_guard<std::mutex> lock(mutexLoadedChunks);
            chunk = world.GetChunk(loadedChunks.front());

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
    return addedChunks;
}
void WorldThreading::addToLoadedChunks(const Chunk* currentChunk) {
    std::lock_guard<std::mutex> lock(mutexChunksToLoadData);
    loadedChunks.push(currentChunk->chunkPosition); //loadedchunks sorts each chunk transparent face
}
void WorldThreading::addToBlocksToBeAdded(const glm::ivec2 tempChunkGlobalPos, const glm::ivec3 tempChunkLocalPos, unsigned char id) {
    std::lock_guard<std::mutex> lock(mutexBlocksToBeAddedList);
    blocksToBeAddedList.emplace_back(
                            tempChunkGlobalPos, tempChunkLocalPos, id
                        );
}

