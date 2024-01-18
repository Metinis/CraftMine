#include "Chunk.h"
#include "World.h"


Chunk::Chunk(glm::ivec2 Position, World& _world) : world(_world)
{
	chunkPosition = Position;
}

Block Chunk::GetBlock(int x, int y, int z)
{
	Block tempBlock(glm::vec3(x + chunkPosition.x * SIZE, y, z + chunkPosition.y * SIZE), BlockIDMap[blockIDs[x + SIZE * (y + HEIGHT * z)]]);
	return tempBlock;
}

unsigned char Chunk::GetBlockID(int x, int y, int z)
{
	return blockIDs[x + SIZE * (y + HEIGHT * z)];
}

void Chunk::SetBlock(int x, int y, int z, unsigned char id)
{
	blockIDs[x + SIZE * (y + HEIGHT * z)] = id;
}

void Chunk::GenBlocks()
{
	float heightMap[HEIGHT * HEIGHT];

	GenChunk(heightMap);

	for (int i = 0; i < SIZE * SIZE * HEIGHT; i++)
	{
		int x = i % SIZE;
		int y = i / (SIZE * SIZE);
		int z = (i / SIZE) % SIZE;

		float amplitude = 1.0f;
		float frequency = 0.01f;
		int columnHeight = HEIGHT;
	
		columnHeight = (int)((heightMap[x + SIZE * z] * 20) * frequency * amplitude * HEIGHT / 2) + (HEIGHT / 2);

		unsigned char id = 0;

		if (y >= columnHeight && y+2 < (HEIGHT / 2)){ //last argument is the sea level
		
			id = 5;
		}
		else if (y == columnHeight - 1)
		{
			id = 1;						//Refer to id map, grass layer
		}
		else if (y > columnHeight - 4 && y < columnHeight)
		{
			id = 2;						//dirt layer
		}
		else if (y <= columnHeight - 4 && y > 0)
		{
			id = 3;						//stone layer
		}
		else if (y == 0)
		{
			id = 4;						//bedrock layer
		}

		SetBlock(x, y, z, id);
	}
    generatedBlockData = true;
}

void Chunk::ClearVertexData()
{
	indexCount = 0;
	chunkVerts.clear();
	chunkUVs.clear();
	chunkIndices.clear();
    generatedBuffData = false;
}

bool Chunk::CheckFace(int x, int y, int z)
{
	if (x >= 0 && x < SIZE && y <= HEIGHT && y >= 0 && z >= 0 && z < SIZE)
	{
		if (BlockIDMap[GetBlockID(x, y, z)] == CraftMine::BlockType::EMPTY)
		{
			return true;
		}
	}
	return false;
}

void Chunk::GenFaces()
{
	ClearVertexData();
	int numFaces = 0;
	for (int x = 0; x < SIZE; x++)
	{
		for (int z = 0; z < SIZE; z++)
		{
			for (int y = 0; y < HEIGHT; y++)
			{
				BlockType type = BlockIDMap[GetBlockID(x, y, z)];
				glm::vec3 blockWorldPos = glm::vec3(x + chunkPosition.x * SIZE, y, z + chunkPosition.y * SIZE);
				if (type != CraftMine::BlockType::EMPTY)	//Will need to be amended for transparent blocks
				{
					//This section could use a cleanup? 

					int leftXoffset = x - 1;

					int rightXoffset = x + 1;

					int frontZoffset = z + 1;

					int backZoffset = z - 1;

					int topYoffset = y + 1;

					int bottomYoffset = y - 1;

					if (CheckFace(leftXoffset, y, z))
					{
						IntegrateFace(Block::GetFace(CraftMine::Faces::LEFT, type, blockWorldPos));
						numFaces++;
					}
					if (CheckFace(rightXoffset, y, z))
					{
						IntegrateFace(Block::GetFace(CraftMine::Faces::RIGHT, type, blockWorldPos));
						numFaces++;
					}
					if (CheckFace(x, y, frontZoffset))
					{
						IntegrateFace(Block::GetFace(CraftMine::Faces::FRONT, type, blockWorldPos));
						numFaces++;
					}
					if (CheckFace(x, y, backZoffset))
					{
						IntegrateFace(Block::GetFace(CraftMine::Faces::BACK, type, blockWorldPos));
						numFaces++;
					}
					if (CheckFace(x, topYoffset, z))
					{
						IntegrateFace(Block::GetFace(CraftMine::Faces::TOP, type, blockWorldPos));
						numFaces++;
					}
					if (CheckFace(x, bottomYoffset, z))
					{
						IntegrateFace(Block::GetFace(CraftMine::Faces::BOTTOM, type, blockWorldPos));
						numFaces++;
					}
				}
			}
		}
	}
	AddIndices(numFaces);
}

void Chunk::UpdateSide(CraftMine::Faces face)
{
	switch (face)
	{
	case CraftMine::LEFT:
		if (chunkPosition.x > 0)
		{
			Chunk& tempChunk = *world.GetChunk(chunkPosition.x - 1, chunkPosition.y);
			if (&tempChunk != nullptr)
			{
				int numFaces = 0;
				for (int y = 0; y < HEIGHT; y++)
				{
					for (int z = 0; z < SIZE; z++)
					{
						
						BlockType type = BlockIDMap[GetBlockID(0, y, z)];
						if (type != CraftMine::EMPTY && BlockIDMap[tempChunk.GetBlockID(SIZE - 1, y, z)] == CraftMine::EMPTY)
						{
							glm::vec3 blockWorldPos = glm::vec3(0 + chunkPosition.x * SIZE, y, z + chunkPosition.y * SIZE);
							IntegrateFace(Block::GetFace(CraftMine::Faces::LEFT, type, blockWorldPos));
							numFaces++;
						}
					}
				}
				AddIndices(numFaces);
			}
		}
		break;
	case CraftMine::RIGHT:
		if (chunkPosition.x < world.SIZE - 1)
		{
			//std::cout << "test";
			Chunk& tempChunk = *world.GetChunk(chunkPosition.x + 1, chunkPosition.y);
			if (&tempChunk != nullptr)
			{
				int numFaces = 0;
				for (int y = 0; y < HEIGHT; y++)
				{
					for (int z = 0; z < SIZE; z++)
					{
						BlockType type = BlockIDMap[GetBlockID(SIZE - 1, y, z)];
						if (type != CraftMine::EMPTY && BlockIDMap[tempChunk.GetBlockID(0, y, z)] == CraftMine::EMPTY)
						{
							glm::vec3 blockWorldPos = glm::vec3((SIZE - 1) + chunkPosition.x * SIZE, y, z + chunkPosition.y * SIZE);
							IntegrateFace(Block::GetFace(CraftMine::Faces::RIGHT, type, blockWorldPos));
							numFaces++;
						}
					}
				}
				AddIndices(numFaces);
			}
		}
		break;
	case CraftMine::FRONT:
		if (chunkPosition.y < world.SIZE - 1)
		{
			//std::cout << "test1";
			Chunk& tempChunk = *world.GetChunk(chunkPosition.x, chunkPosition.y + 1);
			if (&tempChunk != nullptr)
			{
				int numFaces = 0;
				for (int x = 0; x < SIZE; x++)
				{
					for (int y = 0; y < HEIGHT; y++)
					{
						BlockType type = BlockIDMap[GetBlockID(x, y, SIZE - 1)];
						if (type != CraftMine::EMPTY && BlockIDMap[tempChunk.GetBlockID(x, y, 0)] == CraftMine::EMPTY)
						{
							glm::vec3 blockWorldPos = glm::vec3(x + chunkPosition.x * SIZE, y, (SIZE-1) + chunkPosition.y * SIZE);
							IntegrateFace(Block::GetFace(CraftMine::Faces::FRONT, type, blockWorldPos));
							numFaces++;
						}
					}
				}
				AddIndices(numFaces);
			}
		}
		break;
	case CraftMine::BACK:
		if (chunkPosition.y > 0)
		{
			Chunk& tempChunk = *world.GetChunk(chunkPosition.x, chunkPosition.y - 1);
			if (&tempChunk != nullptr)
			{
				int numFaces = 0;
				for (int x = 0; x < SIZE; x++)
				{
					for (int y = 0; y < HEIGHT; y++)
					{
						int numFaces = 0;
						BlockType type = BlockIDMap[GetBlockID(x, y, 0)];
						if (type != CraftMine::EMPTY && BlockIDMap[tempChunk.GetBlockID(x, y, SIZE-1)] == CraftMine::EMPTY)
						{
							glm::vec3 blockWorldPos = glm::vec3(x + chunkPosition.x * SIZE, y, (0) + chunkPosition.y * SIZE);
							IntegrateFace(Block::GetFace(CraftMine::Faces::BACK, type, blockWorldPos));
							numFaces++;
							AddIndices(numFaces);
						}
					}
				}
				AddIndices(numFaces);
			}

		}
		break;
	}
}

void Chunk::UpdateNeighbours()
{
	//update the right side of the left chunk
	if (chunkPosition.x > 0)
	{
		Chunk& tempChunk = *world.GetChunk(chunkPosition.x - 1, chunkPosition.y);
		
		//need to update to wait for chunk to be loaded if in thread, should update in same thread or another to avoid lag
		if (&tempChunk != nullptr && tempChunk.generatedBuffData)
		{
			UpdateSide(CraftMine::LEFT);
			tempChunk.UpdateSide(CraftMine::RIGHT);
			world.loadedChunks.push(&tempChunk);
			
		}
		
	}
	//Update the left side of the right chunk
	if (chunkPosition.x < world.SIZE - 1)
	{
		Chunk& tempChunk = *world.GetChunk(chunkPosition.x + 1, chunkPosition.y);
		if (&tempChunk != nullptr && tempChunk.generatedBuffData)
		{
			UpdateSide(CraftMine::RIGHT);
			tempChunk.UpdateSide(CraftMine::LEFT);
			world.loadedChunks.push(&tempChunk);
			
		}
	}
	//Update back side of the front chunk
	if (chunkPosition.y < world.SIZE - 1)
	{
		Chunk& tempChunk = *world.GetChunk(chunkPosition.x, chunkPosition.y + 1);
		if (&tempChunk != nullptr && tempChunk.generatedBuffData)
		{
			UpdateSide(CraftMine::FRONT);
			tempChunk.UpdateSide(CraftMine::BACK);
			world.loadedChunks.push(&tempChunk);
			
		}
	}
	//Update front side of the back chunk
	if (chunkPosition.y > 0)
	{
		Chunk& tempChunk = *world.GetChunk(chunkPosition.x, chunkPosition.y - 1);
		if (&tempChunk != nullptr && tempChunk.generatedBuffData)
		{
			UpdateSide(CraftMine::BACK);
			tempChunk.UpdateSide(CraftMine::FRONT);
			world.loadedChunks.push(&tempChunk);
			
		}
	}
}

void Chunk::GenChunk(float* heightMap)	//might need to be amended to include more chunks
{
	for (int x = 0; x < SIZE; x++)
	{
		for (int z = 0; z < SIZE; z++)
		{
			float _x = (float)x + (SIZE * (float)chunkPosition.x);
			float _z = (float)z + (SIZE * (float)chunkPosition.y);
			heightMap[x + SIZE * z] = SimplexNoise::noise(_x / 100.0f, _z / 100.0f);	//Don't play with his value, works good as it is
		}
	}
}

void Chunk::IntegrateFace(FaceData faceData)
{
	//FaceData faceData = block.GetFace(face);
	chunkVerts.insert(chunkVerts.end(), faceData.vertices.begin(), faceData.vertices.end());
	chunkUVs.insert(chunkUVs.end(), faceData.texCoords.begin(), faceData.texCoords.end());
}

void Chunk::AddIndices(int amtFaces)
{
	for (int i = 0; i < amtFaces; i++)
	{
		chunkIndices.push_back(0 + indexCount);
		chunkIndices.push_back(1 + indexCount);
		chunkIndices.push_back(2 + indexCount);
		chunkIndices.push_back(2 + indexCount);
		chunkIndices.push_back(3 + indexCount);
		chunkIndices.push_back(0 + indexCount);

		indexCount += 4;	//uses element index, since each face only has 4 indices, we increment this everytime any block face is added
	}
}

void Chunk::ReloadBufferData()
{
	//this part mainly used to update a side if there was a chunk loaded nearby, hence not deleting and recalculating buffers
	if (chunkVertexVBO != nullptr && chunkUVVBO != nullptr && chunkIBO != nullptr && chunkVAO != nullptr)
	{
		chunkVAO->Bind();
		chunkVertexVBO->SetNewData(chunkVerts);
		chunkUVVBO->SetNewData(chunkUVs);
		chunkIBO->SetNewData(chunkIndices);
		chunkVertexVBO->Bind();
		chunkVAO->LinkToVAO(0, 3, *chunkVertexVBO);
		chunkUVVBO->Bind();
		chunkVAO->LinkToVAO(1, 2, *chunkUVVBO);
		chunkVertexVBO->Unbind();
		chunkUVVBO->Unbind();
		chunkVAO->Unbind();
	}
	else
	{
		LoadBufferData();
	}
}

void Chunk::LoadBufferData()
{
	// Reset pointers to nullptr
	Delete();

	// Create new buffers and load data
	chunkVAO = new VAO();
	chunkVAO->Bind();

	chunkVertexVBO = new VBO(chunkVerts);
	chunkVertexVBO->Bind();
	chunkVAO->LinkToVAO(0, 3, *chunkVertexVBO);
	chunkVertexVBO->Unbind();

	chunkUVVBO = new VBO(chunkUVs);
	chunkUVVBO->Bind();
	chunkVAO->LinkToVAO(1, 2, *chunkUVVBO);
	chunkUVVBO->Unbind();

	chunkVAO->Unbind();

	chunkIBO = new IBO(chunkIndices);
	//chunkIBO->Bind();
}

void Chunk::RenderChunk()
{
	chunkVAO->Bind();
	chunkIBO->Bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(chunkIndices.size()), GL_UNSIGNED_INT, 0);
	chunkVAO->Unbind();
	chunkIBO->Unbind();
}

void Chunk::LoadChunkData() {
	GenFaces();
	UpdateNeighbours();
}

void Chunk::Delete()
{
	if (chunkVAO != nullptr) {
        chunkVAO->Delete();
		delete chunkVAO;
		chunkVAO = nullptr;
	}

	if (chunkVertexVBO != nullptr) {
        chunkVertexVBO->Delete();
		delete chunkVertexVBO;
		chunkVertexVBO = nullptr;
	}

	if (chunkUVVBO != nullptr) {
        chunkUVVBO->Delete();
		delete chunkUVVBO;
		chunkUVVBO = nullptr;
	}

	if (chunkIBO != nullptr) {
        chunkIBO->Delete();
		delete chunkIBO;
		chunkIBO = nullptr;
	}
    //ClearVertexData();
}

Chunk::~Chunk()
{
	Delete();
}