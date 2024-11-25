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
#include "WorldThreading.h"

class Player;

class Input;

class Chunk;

class ChunkMeshGeneration;

class Scene;

class World
{
private:
    glm::vec2 playerChunkPos{};



    struct CompareChunks {
        glm::ivec2 _playerChunkPos = glm::ivec2(50,50);

		bool operator()(const glm::ivec2 chunkPos1, const glm::ivec2 chunkPos2 ) const{
			const int x1 = _playerChunkPos.x - chunkPos1.x;
			const int y1 = _playerChunkPos.y - chunkPos1.y;
			const int x2 = _playerChunkPos.x - chunkPos2.x;
			const int y2 = _playerChunkPos.y - chunkPos2.y;
			const double distance1 = sqrt(x1 * x1 + y1 * y1);
			const double distance2 = sqrt(x2 * x2 + y2 * y2);
			return distance1 > distance2;
		}
	};





struct Plane
{
    glm::vec3 normal = { 0.f, 1.f, 0.f }; // unit vector
	float     distance = 0.f;        // Distance with origin

	Plane() = default;

	Plane(const glm::vec3& p1, const glm::vec3& norm)
		: normal(glm::normalize(norm)),
		distance(glm::dot(normal, p1))
	{}

	float getSignedDistanceToPlane(const glm::vec3& point) const
	{
		return glm::dot(normal, point) - distance;
	}  };
struct Frustum
{
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;
};

    bool isChunkInFrustum(const glm::vec3& minCorner, const glm::vec3& maxCorner) const;
    static Frustum createFrustumFromCamera(const Camera& cam, float aspect, float fovY,
                                                                float zNear, float zFar);
    static bool isPointInFrustum(const glm::vec3& point, const Frustum& frustum);
	void LoadThreadDataToMain();

public:
Frustum frustum;
    Player& player;
    Camera& camera;
    Scene& scene;

	static constexpr int SIZE = 1000;
	static int viewDistance;



	std::vector<glm::ivec2> activeChunks;	 //chunks that are currently being rendered, any loaded chunks need to be sent from thread to here
    std::vector<glm::ivec2> chunksToSortFaces;


	std::vector<BlocksToBeAdded> liquidToBeChecked;



    Chunk* chunks[SIZE*SIZE] = {nullptr};

	explicit World(Camera& _camera, Scene& scene, Player& player);


	void UpdateViewDistance(const glm::ivec2& cameraChunkPos);

	void GenerateChunkBuffers(std::vector<Chunk*>& addedChunks);

	Chunk* GetChunk(int x, int y) const;

    Chunk* GetChunk(glm::ivec2 pos) const;

    bool RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& currentChunk) const;

    bool RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& currentChunk, glm::ivec3& lastEmptyPos) const;

    void PlaceBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);

    void BreakBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const;

    void renderChunksToNormalShaders() const;

    void renderSolidMeshes(Shader& shader) const;

    void renderTransparentMeshes(Shader& shader) const;

	void update();

	void updateTick();

    void sortChunks();

    void renderChunksToShader(Shader& shader) const;

	void renderChunksToShadow(Shader& shader) const;

    void sortTransparentFaces();

    void sortChunks(glm::vec3 pos);

    static void saveBlocksToBeAddedToFile();

    static void loadDataFromFile();

	void deleteChunk(glm::ivec2 pos);
};


