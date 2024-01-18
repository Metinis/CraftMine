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
#include "SimplexNoise.h"
//#include "World.h"
#include <algorithm>

class World;

class Chunk
{
private:
	std::vector<glm::vec3> chunkVerts;
	std::vector<glm::vec2> chunkUVs;
	std::vector<GLuint> chunkIndices;
	
	GLsizei indexCount = 0;

	VAO *chunkVAO = nullptr;
	VBO *chunkVertexVBO = nullptr;
	VBO *chunkUVVBO = nullptr;
	IBO *chunkIBO = nullptr;

	World& world;

	bool CheckFace(int x, int y, int z);
	void GenChunk(float* heightMap);
	void IntegrateFace(FaceData faceData);
	void AddIndices(int amtFaces);
	void UpdateNeighbours();
	void GenFaces();
	
	
public:
	static const int SIZE = 16;
	static const int HEIGHT = 128;

    bool generatedBlockData = false;
	bool generatedBuffData = false;
	bool inThread = false;

	unsigned char blockIDs[SIZE * HEIGHT * SIZE];
	glm::ivec2 chunkPosition;

	Chunk(glm::ivec2 Position, World& world);
	~Chunk();
	Block GetBlock(int x, int y, int z);
	unsigned char GetBlockID(int x, int y, int z);
	void SetBlock(int x, int y, int z, unsigned char id);
	void GenBlocks();
	void UpdateSide(CraftMine::Faces face);
	void ClearVertexData();
	//OpenGL stuff
	void LoadChunkData();
	void LoadBufferData();
	void ReloadBufferData();
	void RenderChunk();
	void Delete();
};

