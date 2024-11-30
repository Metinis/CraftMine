#pragma once
#include <glad/glad.h>
#include <vector>
#include <array>
#include "Block.h"
#include "Shader.h"
#include "VBO.h"
#include "Mesh.h"
#include "FastNoise/FastNoise.h"
#include <random>
#include <mutex>
#include "zlib.h"

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
        std::vector<glm::vec4> chunkRGBIValues;
        std::vector<glm::vec3> chunkNormals;

        std::vector<glm::vec3> nonSolidVerts;
        std::vector<glm::vec2> nonSolidUVs;
        std::vector<GLuint> nonSolidIndices;
        std::vector<glm::vec4> nonSolidRGBIValues;
        std::vector<glm::vec3> nonSolidNormals;

        GLsizei indexCount = 0;
        GLsizei nonSolidIndexCount = 0;
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
        std::array<glm::vec4, 4> rgbiLightValues;
        std::array<glm::vec3, 4> normals;
    };

	static constexpr int CHUNKS_PER_REGION = 32;

public:
	static constexpr int SIZE = 24;
	static constexpr int HEIGHT = 192;

    ChunkData chunkData;
    ChunkBools chunkBools;

    std::mutex chunkMeshMutex;
    std::mutex chunkBlockMutex;
    std::mutex chunkDeleteMutex;

    Mesh* mesh = nullptr;
    Mesh* transparentMesh = nullptr;

    bool generatedBlockData = false;
	bool generatedBuffData = false;
    bool chunkHasMeshes = false;
	bool inThread = false;
    bool toBeDeleted = false;

    World& world;

	typedef struct {
		unsigned char blockID;      // Block type
		std::array<unsigned char, 4> light;     // RGBI lighting
		unsigned char orientation;  // Orientation
	} Block;

	Block blocks[SIZE * HEIGHT * SIZE] = {0}; // Array of structured blocks



	glm::ivec2 chunkPosition{};

	Chunk(glm::ivec2 Position, World& _world);
	~Chunk();
    unsigned char GetBlockID(glm::ivec3 pos) const;
	glm::vec4 getBlockLightNormalised(glm::ivec3 pos) const;
	std::array<unsigned char, 4> getBlockLightValue(glm::ivec3 pos) const;
    void SetBlock(glm::ivec3 pos, unsigned char id);
	void SetBlockLighting(glm::ivec3 pos, std::array<unsigned char, 4> rgbaLight);
	void genBlocks();
	void genLight();
	void ClearVertexData();
    static bool compareDistanceToPlayer(const ChunkDataPair& pair1, const ChunkDataPair& pair2, glm::vec3 playerPos);
    void sortTransparentMeshData();
    void sortTransparentMeshData(glm::vec3 position);
	//OpenGL stuff
	void LoadChunkData();
	void LoadBufferData();
    void saveData();
    bool loadData();
	static std::string getRegionFilename(int regionX, int regionY);
	static int getChunkOffset(int chunkX, int chunkY);

	void Delete();
    struct CompareFaces;

    bool getIsAllSidesUpdated();

	glm::vec3 getChunkMinBounds() const;
	glm::vec3 getChunkMaxBounds() const;
};

