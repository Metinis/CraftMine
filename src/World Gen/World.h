#pragma once
#include <glad/glad.h>
#include"Shader.h"
#include<glm/vec3.hpp>
#include"Camera.h"
//#include"Chunk.h"
#include <thread>
#include <future>
#include <queue>
#include <stack>
#include <cmath>

class Chunk;

class World
{
private:
	Camera& camera;
	Shader* shader;
	Texture* texture;

	glm::mat4 model{};
	glm::mat4 view{};
	glm::mat4 proj{};

    glm::vec2 playerChunkPos;

	bool threadFinished = true;

	std::mutex mutexChunksToGenerate;
	std::mutex mutexChunksToLoadData;
	std::mutex mutexChunksToDelete;
	struct CompareChunks {
        glm::ivec2 _playerChunkPos = glm::ivec2(50,50);

		bool operator()(Chunk* chunk1, Chunk* chunk2){
			int x1 = _playerChunkPos.x - chunk1->chunkPosition.x;
			int y1 = _playerChunkPos.y - chunk1->chunkPosition.y;
			int x2 = _playerChunkPos.x - chunk2->chunkPosition.x;
			int y2 = _playerChunkPos.y - chunk2->chunkPosition.y;
			double distance1 = sqrt(x1 * x1 + y1 * y1);
			double distance2 = sqrt(x2 * x2 + y2 * y2);
			return distance1 < distance2;
		}
	};
	void GenerateChunkData(Chunk* chunk);
	void GenerateChunkBuffers(std::vector<Chunk*>& addedChunks);
	void BindPrograms();

public:
	static const int SIZE = 100;
	int viewDistance = 10;
	std::thread chunkThread;
	std::thread worldGenThread;

	std::queue<Chunk*> chunksToGenerate;  //used for world gen for chunks that havent been generated yet -> generates blocks
	std::queue<Chunk*> chunksToLoadData; //used to load faces
	std::queue<Chunk*> loadedChunks;	 //sent to main thread to be assigned buffers
	std::vector<Chunk*> chunksToDelete;
	std::vector<Chunk*> activeChunks;	 //chunks that are currently being rendered, any loaded chunks need to be sent from thread to here
    Chunk* chunks[SIZE*SIZE] = {nullptr};

	World(Camera& camera);

    [[noreturn]] void GenerateChunkThread();

    [[noreturn]] void GenerateWorldThread();
	void UpdateViewDistance(glm::ivec2 cameraPos);

	Chunk* GetChunk(int x, int y);

	void RenderWorld(Camera _camera);
};

