#ifndef WORLDTHREADING_H
#define WORLDTHREADING_H
#include <queue>
#include <vector>
#include "World.h"
#include <unordered_set>
#include "ChunkMeshGeneration.h"

class WorldThreading {
private:
    static std::thread chunkThread;
    static std::thread worldGenThread;

    static std::vector<glm::ivec2> chunksToGenerate;  //used for world gen for chunks that haven't been generated yet -> generates blocks
    static std::vector<glm::ivec2> chunksToLoadData; //used to load faces
    static std::vector<glm::ivec2> chunksToDelete;


    static std::mutex mutexChunksToLoadData;
    static std::mutex mutexLoadedChunks;
    static std::mutex mutexLiquidBlocks;
    static std::mutex mutexChunksToGenerate;



    struct ChunkPosHash {
        std::size_t operator()(const glm::ivec2& pos) const {
            return std::hash<int>()(pos.x) ^ std::hash<int>()(pos.y);
        }
    };

    static void GenerateChunkThread(const World& world);

    static void GenerateWorldThread(const World& world);

    static bool CheckForBlocksToBeAdded(Chunk* chunk);

public:
    static std::mutex mutexBlocksToBeAddedList;
    static std::vector<BlocksToBeAdded> blocksToBeAddedList;
    static std::queue<glm::ivec2> loadedChunks;	 //sent to main thread to be assigned buffers

    static void startThreads(World& world);

    static void deleteInActiveChunks(World& world, const std::vector<glm::ivec2>& chunksLoading, const std::vector<glm::ivec2>& newActiveChunks, const std::vector<glm::ivec2>& activeChunks);

    static void setChunksToGenerate(const std::vector<glm::ivec2>& generateChunks);

    static void setChunksToLoadData(const std::vector<glm::ivec2>& chunksLoading);

    static void updateLoadData(Chunk* currentChunk);

    static std::vector<Chunk*> GetAddedThreadChunks(World& world);

    static void addToLoadedChunks(const Chunk* currentChunk);

    static void addToBlocksToBeAdded(glm::ivec2 tempChunkGlobalPos, glm::ivec3 tempChunkLocalPos, unsigned char id);
};



#endif //WORLDTHREADING_H
