#include "Chunk.h"
#include "World.h"


Chunk::Chunk(glm::ivec2 Position, World& _world) : world(_world)
{
	chunkPosition = Position;
}

unsigned char Chunk::GetBlockID(int x, int y, int z)
{
	return blockIDs[x + SIZE * (y + HEIGHT * z)];
}
Block Chunk::GetBlock(glm::vec3 pos, int id)
{
    Block tempBlock(pos, id);
    return tempBlock;
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

    transparentVerts.clear();
    transparentUVs.clear();
    transparentIndices.clear();
    transparentIndexCount = 0;

    generatedBuffData = false;
}

bool Chunk::CheckFace(int x, int y, int z, bool isSolid)
{
	if (x >= 0 && x < SIZE && y <= HEIGHT && y >= 0 && z >= 0 && z < SIZE)
	{
		if (Block::transparent(GetBlockID(x,y,z)) && isSolid)
		{
			return true;
		}
        else if(Block::transparent(GetBlockID(x,y,z)) && !isSolid && GetBlockID(x,y,z) == 0) //empty
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
    int numTransparentFaces = 0;
	for (int x = 0; x < SIZE; x++)
	{
		for (int z = 0; z < SIZE; z++)
		{
			for (int y = 0; y < HEIGHT; y++)
			{
				glm::vec3 blockWorldPos = glm::vec3(x + chunkPosition.x * SIZE, y, z + chunkPosition.y * SIZE);
				if (!Block::transparent(GetBlockID(x,y,z)))
				{
                    AddFaces(x,y,z, numFaces, true);
				}
                else if(GetBlockID(x,y,z) != 0) //if not empty
                {
                    //add to transparent mesh, integrate face only if bordering empty
                    AddFaces(x,y,z, numTransparentFaces, false);
                }
			}
		}
	}
	AddIndices(numFaces, chunkIndices, indexCount);
    AddIndices(numTransparentFaces, transparentIndices, transparentIndexCount);
}
void Chunk::AddFaces(int x, int y, int z, int &numFaces, bool isSolid) //checks the isSolid faces and adds them
{
    BlockType type = BlockIDMap[GetBlockID(x, y, z)];

    glm::vec3 blockWorldPos = glm::vec3(x + chunkPosition.x * SIZE, y, z + chunkPosition.y * SIZE);

    int leftXoffset = x - 1;

    int rightXoffset = x + 1;

    int frontZoffset = z + 1;

    int backZoffset = z - 1;

    int topYoffset = y + 1;

    int bottomYoffset = y - 1;

    if (CheckFace(leftXoffset, y, z, isSolid))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::LEFT, type, blockWorldPos), isSolid);
        numFaces++;
    }
    if (CheckFace(rightXoffset, y, z, isSolid))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::RIGHT, type, blockWorldPos), isSolid);
        numFaces++;
    }
    if (CheckFace(x, y, frontZoffset, isSolid))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::FRONT, type, blockWorldPos), isSolid);
        numFaces++;
    }
    if (CheckFace(x, y, backZoffset, isSolid))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::BACK, type, blockWorldPos), isSolid);
        numFaces++;
    }
    if (CheckFace(x, topYoffset, z, isSolid))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::TOP, type, blockWorldPos), isSolid);
        numFaces++;
    }
    if (CheckFace(x, bottomYoffset, z, isSolid))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::BOTTOM, type, blockWorldPos), isSolid);
        numFaces++;
    }
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
						//BlockType type = BlockIDMap[GetBlockID(0, y, z)];
						if (!Block::transparent(GetBlockID(0, y, z)) && Block::transparent(tempChunk.GetBlockID(SIZE - 1, y, z)))
						{
							glm::vec3 blockWorldPos = glm::vec3(0 + chunkPosition.x * SIZE, y, z + chunkPosition.y * SIZE);
							IntegrateFace(Block::GetFace(CraftMine::Faces::LEFT, BlockIDMap[GetBlockID(0, y, z)], blockWorldPos), true);
							numFaces++;
						}
					}
				}
				AddIndices(numFaces, chunkIndices, indexCount);
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
						//BlockType type = BlockIDMap[GetBlockID(SIZE - 1, y, z)];
                        if (!Block::transparent(GetBlockID(SIZE-1, y, z)) && Block::transparent(tempChunk.GetBlockID(0, y, z)))
						{
							glm::vec3 blockWorldPos = glm::vec3((SIZE - 1) + chunkPosition.x * SIZE, y, z + chunkPosition.y * SIZE);
							IntegrateFace(Block::GetFace(CraftMine::Faces::RIGHT, BlockIDMap[GetBlockID(SIZE - 1, y, z)], blockWorldPos), true);
							numFaces++;
						}
					}
				}
				AddIndices(numFaces, chunkIndices, indexCount);
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
						//BlockType type = BlockIDMap[GetBlockID(x, y, SIZE - 1)];
                        if (!Block::transparent(GetBlockID(x, y, SIZE - 1)) && Block::transparent(tempChunk.GetBlockID(x, y, 0)))
						{
							glm::vec3 blockWorldPos = glm::vec3(x + chunkPosition.x * SIZE, y, (SIZE-1) + chunkPosition.y * SIZE);
							IntegrateFace(Block::GetFace(CraftMine::Faces::FRONT, BlockIDMap[GetBlockID(x, y, SIZE - 1)], blockWorldPos), true);
							numFaces++;
						}
					}
				}
				AddIndices(numFaces, chunkIndices, indexCount);
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
						//BlockType type = BlockIDMap[GetBlockID(x, y, 0)];
                        if (!Block::transparent(GetBlockID(x, y, 0)) && Block::transparent(tempChunk.GetBlockID(x, y, SIZE - 1)))
						{
							glm::vec3 blockWorldPos = glm::vec3(x + chunkPosition.x * SIZE, y, (0) + chunkPosition.y * SIZE);
							IntegrateFace(Block::GetFace(CraftMine::Faces::BACK, BlockIDMap[GetBlockID(x, y, 0)], blockWorldPos), true);
							numFaces++;
							//AddIndices(numFaces, chunkIndices);
						}
					}
				}
				AddIndices(numFaces, chunkIndices, indexCount);
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
void Chunk::IntegrateFace(FaceData faceData, bool solid)
{
    //FaceData faceData = block.GetFace(face);
    if(solid) {
        chunkVerts.insert(chunkVerts.end(), faceData.vertices.begin(), faceData.vertices.end());
        chunkUVs.insert(chunkUVs.end(), faceData.texCoords.begin(), faceData.texCoords.end());
    }
    else
    {
        transparentVerts.insert(transparentVerts.end(), faceData.vertices.begin(), faceData.vertices.end());
        transparentUVs.insert(transparentUVs.end(), faceData.texCoords.begin(), faceData.texCoords.end());
    }
}

void Chunk::AddIndices(int amtFaces, std::vector<GLuint> &indices, GLsizei &_indexCount)
{
	for (int i = 0; i < amtFaces; i++)
	{
		indices.push_back(0 + _indexCount);
        indices.push_back(1 + _indexCount);
        indices.push_back(2 + _indexCount);
        indices.push_back(2 + _indexCount);
        indices.push_back(3 + _indexCount);
        indices.push_back(0 + _indexCount);

        _indexCount += 4;	//uses element index, since each face only has 4 indices, we increment this everytime any block face is added
	}
}

void Chunk::LoadBufferData()
{
    if(mesh != nullptr)
    {
        delete mesh;
    }
    mesh = new Mesh(world.shader);
    mesh->setData(chunkVerts, chunkUVs, chunkIndices);
	mesh->loadData();

    if(transparentMesh != nullptr)
    {
        delete transparentMesh;
    }
    transparentMesh = new Mesh(world.transparentShader);
    transparentMesh->setData(transparentVerts, transparentUVs, transparentIndices);
    transparentMesh->loadData();
}

void Chunk::RenderChunk()
{

    //TODO fix rendering of transparent mesh/shader
    world.shader->use();
    mesh->render();
    transparentMesh->render();
}

void Chunk::LoadChunkData() {
	GenFaces();
	UpdateNeighbours();
}

void Chunk::Delete()
{
    ClearVertexData();
    delete mesh;
    mesh = nullptr;

    delete transparentMesh;
    transparentMesh = nullptr;
}

Chunk::~Chunk()
{
	Delete();
}