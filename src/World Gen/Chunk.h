#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <vector>
#include <array>
#include <map>
#include "Block.h"
#include "Shader.h"
#include "VBO.h"
#include "VAO.h"
#include "IBO.h"
#include "Texture.h"
#include <algorithm>
#include "Mesh.h"
#include "FastNoise/FastNoise.h"
#include <random>
#include "BlockData.h"
#include <mutex>

class Player;

class ChunkGeneration;

class World;

class ChunkMeshGeneration;

class Chunk
{
private:
    bool isLoadingData = false;
    struct ChunkData{
        std::vector<glm::vec3> chunkVerts;
        std::vector<glm::vec2> chunkUVs;
        std::vector<GLuint> chunkIndices;
        std::vector<float> chunkBrightnessFloats;
        std::vector<glm::vec3> chunkNormals;

        std::vector<glm::vec3> transparentVerts;
        std::vector<glm::vec2> transparentUVs;
        std::vector<GLuint> transparentIndices;
        std::vector<float> transparentBrightnessFloats;
        std::vector<glm::vec3> transparentNormals;

        GLsizei indexCount = 0;
        GLsizei transparentIndexCount = 0;
    };
    struct ChunkBools{
        bool leftSideUpdated = false;
        bool rightSideUpdated = false;
        bool frontUpdated = false;
        bool backUpdated = false;
    };
    struct ChunkDataPair{
        std::array<glm::vec3, 4> vertices;
        std::array<glm::vec2, 4> uvs;
        std::array<GLuint, 6> indices;
        std::array<float, 4> brightnessFloats;
        std::array<glm::vec3, 4> normals;
    };



	
public:
	static const int SIZE = 16;
	static const int HEIGHT = 256;

    ChunkData chunkData;
    ChunkBools chunkBools;

    std::mutex chunkMutex;

    Mesh* mesh = nullptr;
    Mesh* transparentMesh = nullptr;

    bool generatedBlockData = false;
	bool generatedBuffData = false;
    bool chunkHasMeshes = false;
	bool inThread = false;

    World& world;

	unsigned char blockIDs[SIZE * HEIGHT * SIZE] = {0}; //initialise all to empty block
	glm::ivec2 chunkPosition;

	Chunk(glm::ivec2 Position, World& world);
	~Chunk();
    unsigned char GetBlockID(glm::ivec3 pos);
    void SetBlock(glm::ivec3 pos, unsigned char id);
	void GenBlocks();
	void ClearVertexData();
    static bool compareDistanceToPlayer(const ChunkDataPair& pair1, const ChunkDataPair& pair2, glm::vec3 playerPos);
    void sortTransparentMeshData();
    void sortTransparentMeshData(glm::vec3 position);
	//OpenGL stuff
	void LoadChunkData();
	void LoadBufferData();
	//void RenderChunk();
    //void RenderShadowChunk(Shader& _shader);
	void Delete();
    struct CompareFaces;

    bool getIsAllSidesUpdated();
};

