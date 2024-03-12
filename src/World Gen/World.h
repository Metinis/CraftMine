#pragma once
#include <glad/glad.h>
#include"Shader.h"
#include<glm/vec3.hpp>
#include"Input/Camera.h"
#include <thread>
#include <future>
#include <queue>
#include <stack>
#include <cmath>
#include <iostream>
#include <filesystem>

class Player;

class MouseInput;

class Chunk;

class ChunkMeshGeneration;

class World
{
private:

	glm::mat4 model{};
	glm::mat4 view{};
	glm::mat4 proj{};

    glm::vec2 playerChunkPos{};

	bool threadFinished = true;

    int lastTexture = 1;
    int lastTime = 0;

    glm::ivec3 lastOutlinePos;

	std::mutex mutexChunksToGenerate;


    struct CompareChunks {
        glm::ivec2 _playerChunkPos = glm::ivec2(50,50);

		bool operator()(Chunk* chunk1, Chunk* chunk2){
			int x1 = _playerChunkPos.x - chunk1->chunkPosition.x;
			int y1 = _playerChunkPos.y - chunk1->chunkPosition.y;
			int x2 = _playerChunkPos.x - chunk2->chunkPosition.x;
			int y2 = _playerChunkPos.y - chunk2->chunkPosition.y;
			double distance1 = sqrt(x1 * x1 + y1 * y1);
			double distance2 = sqrt(x2 * x2 + y2 * y2);
			return distance1 > distance2;
		}
	};

    void CheckForBlocksToBeAdded(Chunk* chunk);
	void GenerateChunkBuffers(std::vector<Chunk*>& addedChunks);
	void LoadShader(Shader* shader);
    void ChangeGlobalTexture();
    void UpdateOutlineBuffers(glm::ivec3& globalPos);
    void DrawOutline() const;
    void UpdateShaders();
    void LoadThreadDataToMain();
    void SortAndRenderChunks();


public:
    Player& player;
    Camera& camera;
    Shader* shader;
    Shader* transparentShader;
    Shader* outlineShader;

    Texture* texture;

    VAO* outlineVAO = nullptr;
    VBO* outlineVBO = nullptr;
    IBO* outlineIBO = nullptr;

	static const int SIZE = 1000;
	int viewDistance = 12;
	std::thread chunkThread;
	std::thread worldGenThread;

	std::vector<Chunk*> chunksToGenerate;  //used for world gen for chunks that haven't been generated yet -> generates blocks
	std::vector<Chunk*> chunksToLoadData; //used to load faces
	std::queue<Chunk*> loadedChunks;	 //sent to main thread to be assigned buffers
	std::vector<Chunk*> chunksToDelete;
	std::vector<Chunk*> activeChunks;	 //chunks that are currently being rendered, any loaded chunks need to be sent from thread to here
    std::vector<Chunk*> chunksToSortFaces;
    std::vector<BlocksToBeAdded> blocksToBeAddedList;

    std::mutex mutexBlocksToBeAddedList;
    std::mutex mutexChunksToLoadData;

    Chunk* chunks[SIZE*SIZE] = {nullptr};

	explicit World(Camera& _camera, Player& _player);

    [[noreturn]] void GenerateChunkThread();

    [[noreturn]] void GenerateWorldThread();

	void UpdateViewDistance(glm::ivec2 cameraPos);

	Chunk* GetChunk(int x, int y);

    bool RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& chunk);

    bool RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& currentChunk, glm::ivec3& lastEmptyPos);

    void PlaceBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);

    void BreakBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);

    void RenderBlockOutline();

	void RenderWorld();

    void RenderShadowWorld(Shader* _shader);
};


