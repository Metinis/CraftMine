#pragma once
#include <glad/glad.h>
#include"Shader.h"
#include<glm/vec3.hpp>
#include"Input/Camera.h"
#include "Scene.h"
#include <thread>
#include <future>
#include <queue>
#include <stack>
#include <cmath>
#include <iostream>
#include <filesystem>
#include <unordered_map>

class Player;

class Input;

class Chunk;

class ChunkMeshGeneration;

class Scene;

class World
{
private:
    glm::vec2 playerChunkPos{};

	std::mutex mutexChunksToGenerate;

    struct CompareChunks {
        glm::ivec2 _playerChunkPos = glm::ivec2(50,50);

		bool operator()(glm::ivec2 chunkPos1, glm::ivec2 chunkPos2 ){
			int x1 = _playerChunkPos.x - chunkPos1.x;
			int y1 = _playerChunkPos.y - chunkPos1.y;
			int x2 = _playerChunkPos.x - chunkPos2.x;
			int y2 = _playerChunkPos.y - chunkPos2.y;
			double distance1 = sqrt(x1 * x1 + y1 * y1);
			double distance2 = sqrt(x2 * x2 + y2 * y2);
			return distance1 > distance2;
		}
	};
    struct ChunkPosHash {
        std::size_t operator()(const glm::ivec2& pos) const {
            return std::hash<int>()(pos.x) ^ std::hash<int>()(pos.y);
        }
    };

    bool CheckForBlocksToBeAdded(Chunk* chunk);
	void GenerateChunkBuffers(std::vector<Chunk*>& addedChunks);
    void LoadThreadDataToMain();

public:
    Player& player;
    Camera& camera;
    Scene& scene;

	static const int SIZE = 1000;
	static int viewDistance;
	std::thread chunkThread;
	std::thread worldGenThread;

	std::vector<glm::ivec2> chunksToGenerate;  //used for world gen for chunks that haven't been generated yet -> generates blocks
	std::vector<glm::ivec2> chunksToLoadData; //used to load faces
	std::queue<glm::ivec2> loadedChunks;	 //sent to main thread to be assigned buffers
	std::vector<glm::ivec2> chunksToDelete;
	std::vector<glm::ivec2> activeChunks;	 //chunks that are currently being rendered, any loaded chunks need to be sent from thread to here
    std::vector<glm::ivec2> chunksToSortFaces;

    std::vector<BlocksToBeAdded> blocksToBeAddedList;

    std::mutex mutexBlocksToBeAddedList;
    std::mutex mutexChunksToLoadData;

    Chunk* chunks[SIZE*SIZE] = {nullptr};

	explicit World(Camera& _camera, Scene& scene, Player& player);

    void GenerateChunkThread();

    void GenerateWorldThread();

	void UpdateViewDistance(glm::ivec2& cameraPos);

	Chunk* GetChunk(int x, int y);

    Chunk* GetChunk(glm::ivec2 pos);

    bool RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& chunk);

    bool RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& currentChunk, glm::ivec3& lastEmptyPos);

    void PlaceBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);

    void BreakBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);

    void renderChunks();

    void renderSolidMeshes(Shader& shader);

    void renderTransparentMeshes(Shader& shader);

	void update();

    void sortChunks();

    void renderChunks(Shader& shader);

    void sortTransparentFaces();

    void sortChunks(glm::vec3 pos);

    void renderChunks(Shader &shader, glm::vec3 lightPos);

    void saveDataToFile();

    void loadDataFromFile();
};


