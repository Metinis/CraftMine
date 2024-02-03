#include "Chunk.h"
#include "World.h"


Chunk::Chunk(glm::ivec2 Position, World& _world) : world(_world)
{
	chunkPosition = Position;
}

unsigned char Chunk::GetBlockID(glm::ivec3 pos)
{
	return blockIDs[pos.x + SIZE * (pos.y + HEIGHT * pos.z)];
}

void Chunk::SetBlock(glm::ivec3 pos, unsigned char id)
{
    if(pos.x < 0 || pos.x > SIZE - 1 || pos.y < 0 || pos.y > HEIGHT - 1 || pos.z < 0 || pos.z > SIZE - 1)
    {
        std::cout<<"invalid block position at: "<<pos.x<<"x "<<pos.y<<"y "<<pos.z<<"z ";
    }
    else {
        blockIDs[pos.x + SIZE * (pos.y + HEIGHT * pos.z)] = id;
    }
}
bool Chunk::shouldGenTree()
{

    std::mt19937 rng(std::random_device{}());
    // Define the probability (1 in 5000 chance)
    std::uniform_int_distribution<> dis(1, 500);

    // Generate a random number
    int randomNumber = dis(rng);

    // Check if the random number falls within the desired range
    return randomNumber == 1;
}
void Chunk::genTree(glm::ivec3 treeCoord) //starts from bottom block
{
    int treeHeight = 5;
    int leafHeight = 2;

    for(int y = treeCoord.y; y < (treeHeight + treeCoord.y + leafHeight) && y < HEIGHT; y++)
    {
        int localY = y - treeCoord.y;
        //set the tree
        if(y < treeHeight + treeCoord.y)
        SetBlock(glm::ivec3(treeCoord.x, y, treeCoord.z), 7); //7 is wood

        if(localY > 2 && localY < treeHeight) {
            //bottom layer
            int startX = treeCoord.x - 2;
            int endX = treeCoord.x + 3;
            int startZ = treeCoord.z - 2;
            int endZ = treeCoord.z + 3;

            generateLeaves(startX, endX, startZ, endZ, y);
        }
            //top layer
        else if(localY >= treeHeight && localY < (treeHeight + leafHeight))
        {
            int startX = treeCoord.x - 1;
            int endX = treeCoord.x + 2;
            int startZ = treeCoord.z - 1;
            int endZ = treeCoord.z + 2;

            generateLeaves(startX, endX, startZ, endZ, y);
        }



    }
}
void Chunk::generateLeaves(int startX, int endX, int startZ, int endZ, int y)
{
    for (int x = startX; x < endX; x++) {
        for (int z = startZ; z < endZ; z++) {
            //if x and z are in this chunk
            if(x >= 0 && x < SIZE && z >= 0 && z < SIZE)
            {
                if (GetBlockID(glm::ivec3(x,y,z)) == 0)
                    SetBlock(glm::ivec3(x,y,z), 8);
            }
            else
            {
                //find the chunk x and z are in, check if it is not null, convert x and y to that local chunk coordinates and set blocks and update buffers
                //Add to loaded chunks for updating
                int tempChunkX;
                int tempChunkZ;

                if(x < 0)
                {
                    tempChunkX = chunkPosition.x - 1;
                }
                else if(x > SIZE - 1)
                {
                    tempChunkX = chunkPosition.x + 1;
                }
                else
                {
                    tempChunkX = chunkPosition.x;
                }
                if(z < 0)
                {
                    tempChunkZ = chunkPosition.y - 1;
                }
                else if(z > SIZE - 1)
                {
                    tempChunkZ = chunkPosition.y + 1;
                }
                else
                {
                    tempChunkZ = chunkPosition.y;
                }
                //std::cout<<tempChunkX<<"x " <<tempChunkZ<<"z "<<"\n";

                if(tempChunkX > 0 && tempChunkX < World::SIZE && tempChunkZ > 0 && tempChunkZ < World::SIZE)
                {

                    Chunk& tempChunk = *world.GetChunk(tempChunkX, tempChunkZ);
                    int tempChunkLocalX;
                    int tempChunkLocalZ;

                    if(x < 0)
                    {
                        tempChunkLocalX = SIZE + x;
                    }
                    else if(x > 15)
                    {
                        tempChunkLocalX = x - SIZE;
                    }
                    else
                    {
                        tempChunkLocalX = x;
                    }
                    if(z < 0)
                    {
                        tempChunkLocalZ = SIZE + z;
                    }
                    else if (z > 15)
                    {
                        tempChunkLocalZ = z - SIZE;
                    }
                    else
                    {
                        tempChunkLocalZ = z;
                    }
                    if(&tempChunk != nullptr && tempChunk.generatedBlockData && !tempChunk.inThread && tempChunk.generatedBuffData)
                    {
                        if(tempChunk.GetBlockID(glm::ivec3(tempChunkLocalX,y,tempChunkLocalZ)) == 0) {
                            //tempChunk.generatedBuffData = false;
                            //tempChunk.generatedBlockData = false;
                            tempChunk.SetBlock(glm::ivec3(tempChunkLocalX,y,tempChunkLocalZ), 8);
                            world.chunksToLoadData.push_back(&tempChunk);
                            //tempChunk.LoadChunkData();
                            //tempChunk.generatedBuffData = true;
                            //tempChunk.generatedBlockData = true;
                            //world.loadedChunks.push(&tempChunk);

                        }
                    }
                    else if(&tempChunk != nullptr && tempChunk.generatedBlockData && !tempChunk.inThread && !tempChunk.generatedBuffData)
                    {
                        if(tempChunk.GetBlockID(glm::ivec3(tempChunkLocalX,y,tempChunkLocalZ)) == 0) {
                            tempChunk.SetBlock(glm::ivec3(tempChunkLocalX,y,tempChunkLocalZ), 8);
                        }
                    }
                    else if(&tempChunk == nullptr || (&tempChunk != nullptr && (!tempChunk.inThread || (tempChunk.inThread && tempChunk.generatedBlockData))))
                    {
                        world.mutexBlocksToBeAddedList.lock();
                        world.blocksToBeAddedList.push_back(BlocksToBeAdded{glm::ivec2(tempChunkX,tempChunkZ), glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ), 8});
                        world.mutexBlocksToBeAddedList.unlock();
                    }

                }

            }

        }
    }
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

	    int seaLevel = HEIGHT / 2;

        int columnHeight = (int)heightMap[x + SIZE * z] + (seaLevel -20); //add sealevel so noise generates above it

		unsigned char id = 0;
        if(GetBlockID(glm::ivec3(x,y,z)) == 0) {    //so that trees/structures don't get overwritten
            if (y >= columnHeight && y + 2 < seaLevel) { //last argument is the sea level

                id = 5;
            } else if (y <= columnHeight - 1 && y >= columnHeight - 3 && y < seaLevel - 2) {
                id = 6; //sand
            } else if (y == columnHeight - 1) {
                id = 1;                        //Refer to id map, grass layer
                //check to generate tree
                if (shouldGenTree()) {
                    id = 2; //if there's tree above, make dirt block, not grass
                    genTree(glm::vec3(x, y+1, z));
                }
            } else if (y > columnHeight - 4 && y < columnHeight) {
                id = 2;                        //dirt layer
            } else if (y <= columnHeight - 4 && y > 0) {
                id = 3;                        //stone layer
            } else if (y == 0) {
                id = 4;                        //bedrock layer
            }

            SetBlock(glm::ivec3(x,y,z), id);
        }
	}
    generatedBlockData = true;
}

void Chunk::ClearVertexData()
{
	indexCount = 0;
	chunkVerts.clear();
	chunkUVs.clear();
	chunkIndices.clear();
    chunkBrightnessFloats.clear();

    transparentVerts.clear();
    transparentUVs.clear();
    transparentIndices.clear();
    transparentIndexCount = 0;
    transparentBrightnessFloats.clear();

    generatedBuffData = false;
}

bool Chunk::CheckFace(int x, int y, int z, bool isSolid)
{
	if (x >= 0 && x < SIZE && y <= HEIGHT && y >= 0 && z >= 0 && z < SIZE)
	{
		if (Block::transparent(GetBlockID(glm::ivec3(x,y,z))) && isSolid)
		{
			return true;
		}
        else if(Block::transparent(GetBlockID(glm::ivec3(x,y,z))) && !isSolid && GetBlockID(glm::ivec3(x,y,z)) == 0) //empty
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
				if (!Block::transparent(GetBlockID(glm::ivec3(x,y,z))))
				{
                    AddFaces(x,y,z, numFaces, true);
				}
                else if(GetBlockID(glm::ivec3(x,y,z)) != 0) //if not empty
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
    BlockType type = BlockIDMap[GetBlockID(glm::ivec3(x,y,z))];

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
    //TODO Clean this up
    Chunk* tempChunk = nullptr;
    int numFaces = 0;
    int numTransparentFaces = 0;

    //Used for neighbouring blocks, we loop through the edges of chunks and compare these two
    //e.g. our block could be in the start edge meaning neighbouring one will be in the endEdge

    int startEdge = 0;
    int endEdge = SIZE - 1;

	switch (face)
	{
	case CraftMine::LEFT:
			tempChunk = world.GetChunk(chunkPosition.x - 1, chunkPosition.y);
            for (int y = 0; y < HEIGHT; y++)
            {
                for (int z = 0; z < SIZE; z++)
                {
                    leftSideUpdated = true;
                    AddEdgeFaces(glm::ivec3(startEdge, y, z), numFaces, numTransparentFaces, z, endEdge, tempChunk, CraftMine::LEFT);
                }
            }
		break;
	case CraftMine::RIGHT:
			tempChunk = world.GetChunk(chunkPosition.x + 1, chunkPosition.y);
            for (int y = 0; y < HEIGHT; y++)
            {
                for (int z = 0; z < SIZE; z++)
                {
                    rightSideUpdated = true;
                    AddEdgeFaces(glm::ivec3(endEdge, y, z), numFaces, numTransparentFaces, z, startEdge, tempChunk, CraftMine::RIGHT);
                }
            }
		break;
	case CraftMine::FRONT:
			tempChunk = world.GetChunk(chunkPosition.x, chunkPosition.y + 1);
            for (int x = 0; x < SIZE; x++)
            {
                for (int y = 0; y < HEIGHT; y++)
                {
                    frontUpdated = true;
                    AddEdgeFaces(glm::ivec3(x, y, endEdge), numFaces, numTransparentFaces, startEdge, x, tempChunk, CraftMine::FRONT);
                }
            }
		break;
	case CraftMine::BACK:
			tempChunk = world.GetChunk(chunkPosition.x, chunkPosition.y - 1);
            for (int x = 0; x < SIZE; x++)
            {
                for (int y = 0; y < HEIGHT; y++)
                {
                    backUpdated = true;
                    AddEdgeFaces(glm::ivec3(x, y, startEdge), numFaces, numTransparentFaces, endEdge, x, tempChunk, CraftMine::BACK);
                }
            }
		break;
	}
    AddIndices(numFaces, chunkIndices, indexCount);
    AddIndices(numTransparentFaces, transparentIndices, transparentIndexCount);
}
void Chunk::AddEdgeFaces(glm::ivec3 localBlockPos, int &numFaces, int &numTransparentFaces, int neighbourZ, int neighbourX, Chunk* tempChunk, Faces face) //used to add the faces neighbouring chunks
{
    glm::vec3 blockWorldPos = glm::vec3(localBlockPos.x + chunkPosition.x * SIZE, localBlockPos.y, localBlockPos.z + chunkPosition.y * SIZE);

    unsigned char blockID = GetBlockID(localBlockPos);

    unsigned char neighbourBlockID = tempChunk->GetBlockID(glm::ivec3(neighbourX, localBlockPos.y, neighbourZ));

    BlockType type = BlockIDMap[blockID];

    if (!Block::transparent(blockID) && Block::transparent(neighbourBlockID))
    {
        IntegrateFace(Block::GetFace(face, type, blockWorldPos), true);
        numFaces++;
    }
    else if(Block::transparent(blockID) && blockID != 0 && neighbourBlockID == 0)
    {
        IntegrateFace(Block::GetFace(face, type, blockWorldPos), false);
        numTransparentFaces++;
    }
}

void Chunk::UpdateNeighbours()
{
	//update the right side of the left chunk
	if (chunkPosition.x > 0)
	{
		Chunk* tempChunk = world.GetChunk(chunkPosition.x - 1, chunkPosition.y);

        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData)
        {
            UpdateSide(CraftMine::LEFT);
            if(!tempChunk->frontUpdated)
            {
                tempChunk->UpdateSide(CraftMine::RIGHT);
                tempChunk->generatedBuffData = false;
                world.loadedChunks.push(tempChunk);
            }
        }
        else if (tempChunk != nullptr && tempChunk->generatedBlockData)
        {
            UpdateSide(CraftMine::LEFT);
        }
	}
	//Update the left side of the right chunk
	if (chunkPosition.x < World::SIZE - 1)
	{
		Chunk* tempChunk = world.GetChunk(chunkPosition.x + 1, chunkPosition.y);
        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData)
        {
            UpdateSide(CraftMine::RIGHT);
            if(!tempChunk->frontUpdated)
            {
                tempChunk->UpdateSide(CraftMine::LEFT);
                tempChunk->generatedBuffData = false;
                world.loadedChunks.push(tempChunk);
            }
        }
        else if (tempChunk != nullptr && tempChunk->generatedBlockData)
        {
            UpdateSide(CraftMine::RIGHT);
        }
	}
	//Update back side of the front chunk
	if (chunkPosition.y < World::SIZE - 1)
	{
		Chunk* tempChunk = world.GetChunk(chunkPosition.x, chunkPosition.y + 1);
        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData)
        {
            UpdateSide(CraftMine::FRONT);
            if(!tempChunk->backUpdated)
            {
                tempChunk->UpdateSide(CraftMine::BACK);
                tempChunk->generatedBuffData = false;
                world.loadedChunks.push(tempChunk);
            }
        }
        else if (tempChunk != nullptr && tempChunk->generatedBlockData)
        {
            UpdateSide(CraftMine::FRONT);
        }

	}
	//Update front side of the back chunk
	if (chunkPosition.y > 0)
	{
		Chunk* tempChunk = world.GetChunk(chunkPosition.x, chunkPosition.y - 1);
        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData)
        {
            UpdateSide(CraftMine::BACK);
            if(!tempChunk->frontUpdated)
            {
                tempChunk->UpdateSide(CraftMine::FRONT);
                tempChunk->generatedBuffData = false;
                world.loadedChunks.push(tempChunk);
            }
        }
        else if (tempChunk != nullptr && tempChunk->generatedBlockData)
        {
            UpdateSide(CraftMine::BACK);
        }
	}
}

void Chunk::GenChunk(float* heightMap)	//might need to be amended to include more chunks
{
    FastNoise noise;

    // Set noise parameters
    noise.SetNoiseType(FastNoise::SimplexFractal);
    noise.SetFractalType(FastNoise::FBM);
    noise.SetFractalOctaves(4);
    noise.SetSeed(123);
    noise.SetFractalLacunarity(2.0f);
    noise.SetFrequency(0.0075f);
    float scaleFactor = 0.25f;

	for (int x = 0; x < SIZE; x++)
	{
		for (int z = 0; z < SIZE; z++)
		{

            float noiseValue = noise.GetNoise((float)x + (SIZE * (float)chunkPosition.x), (float)z + (SIZE * (float)chunkPosition.y));
            int terrainHeight = static_cast<int>((noiseValue + 1.0) * 100.0f * scaleFactor); //100.0f represents the highest point
            heightMap[x + SIZE * z] = terrainHeight;
		}
	}
}
void Chunk::IntegrateFace(FaceData faceData, bool solid)
{
    //FaceData faceData = block.GetFace(face);
    if(solid) {
        chunkVerts.insert(chunkVerts.end(), faceData.vertices.begin(), faceData.vertices.end());
        chunkUVs.insert(chunkUVs.end(), faceData.texCoords.begin(), faceData.texCoords.end());
        //each vert has 4 points so need to repeat 4 times, can be adjusted to make nicer lighting later
        for(int i = 0; i < 4; i++)
        {
            chunkBrightnessFloats.push_back(faceData.brightness);
        }
    }
    else
    {
        transparentVerts.insert(transparentVerts.end(), faceData.vertices.begin(), faceData.vertices.end());
        transparentUVs.insert(transparentUVs.end(), faceData.texCoords.begin(), faceData.texCoords.end());
        for(int i = 0; i < 4; i++)
        {
            transparentBrightnessFloats.push_back(faceData.brightness);
        }

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
    mesh = new Mesh(*world.shader);
    mesh->setData(chunkVerts, chunkUVs, chunkIndices, chunkBrightnessFloats);
	mesh->loadData();

    if(transparentMesh != nullptr)
    {
        delete transparentMesh;
    }
    transparentMesh = new Mesh(*world.transparentShader);
    transparentMesh->setData(transparentVerts, transparentUVs, transparentIndices, transparentBrightnessFloats);
    transparentMesh->loadData();
}

void Chunk::RenderChunk()
{
    if(mesh != nullptr && transparentMesh != nullptr) {
        mesh->render();
        transparentMesh->render();
    }
}

void Chunk::LoadChunkData() {
    ClearVertexData();
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
