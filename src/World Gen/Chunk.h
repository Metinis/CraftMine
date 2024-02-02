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

class World;

class Chunk
{
private:
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

    Mesh* mesh = nullptr;
    Mesh* transparentMesh = nullptr;

	World& world;

	bool CheckFace(int x, int y, int z, bool isSolid);
    void AddFaces(int x, int y, int z, int &numFaces, bool isSolid);
    static bool shouldGenTree();
    void genTree(glm::ivec3 treeCoord);
	void GenChunk(float* heightMap);
    void IntegrateFace(FaceData faceData, bool isTransparent);
    void AddIndices(int amtFaces, std::vector<GLuint> &indices, GLsizei &_indexCount);
    void AddEdgeFaces(glm::ivec3 localBlockPos, int &numFaces, int &numTransparentFaces, int neighbourZ, int neighbourX, Chunk* tempChunk, Faces face);
	void UpdateNeighbours();
	void GenFaces();
	
	
public:
	static const int SIZE = 16;
	static const int HEIGHT = 128;

    bool generatedBlockData = false;
	bool generatedBuffData = false;
	bool inThread = false;


	unsigned char blockIDs[SIZE * HEIGHT * SIZE] = {0}; //initialise all to empty block
	glm::ivec2 chunkPosition;

	Chunk(glm::ivec2 Position, World& world);
	~Chunk();
	unsigned char GetBlockID(int x, int y, int z);
    Block GetBlock(glm::vec3 pos, int id);
	void SetBlock(int x, int y, int z, unsigned char id);
	void GenBlocks();
	void UpdateSide(CraftMine::Faces face);
	void ClearVertexData();
	//OpenGL stuff
	void LoadChunkData();
	void LoadBufferData();
	void RenderChunk();
	void Delete();
};

