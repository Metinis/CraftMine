#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <vector>
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

class ChunkGeneration;

class World;

class ChunkMeshGeneration;

class Chunk
{
private:
    Mesh* mesh = nullptr;
    Mesh* transparentMesh = nullptr;

    struct ChunkData{
        std::vector<glm::vec3> chunkVerts;
        std::vector<glm::vec2> chunkUVs;
        std::vector<GLuint> chunkIndices;
        std::vector<float> chunkBrightnessFloats;

        std::vector<glm::vec3> transparentVerts;
        std::vector<glm::vec2> transparentUVs;
        std::vector<GLuint> transparentIndices;
        std::vector<float> transparentBrightnessFloats;

        GLsizei indexCount = 0;
        GLsizei transparentIndexCount = 0;
    };
    struct ChunkBools{
        bool leftSideUpdated = false;
        bool rightSideUpdated = false;
        bool frontUpdated = false;
        bool backUpdated = false;
    };
	
public:
	static const int SIZE = 16;
	static const int HEIGHT = 128;

    ChunkData chunkData;
    ChunkBools chunkBools;

    bool generatedBlockData = false;
	bool generatedBuffData = false;
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
	//OpenGL stuff
	void LoadChunkData();
	void LoadBufferData();
	void RenderChunk();
	void Delete();
};

