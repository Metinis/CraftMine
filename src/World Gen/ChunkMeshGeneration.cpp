//
// Created by vodkasas on 03/02/24.
//

#include "ChunkMeshGeneration.h"
bool ChunkMeshGeneration::CheckFace(int x, int y, int z, bool isSolid, Chunk& chunk)
{
    if (x >= 0 && x < Chunk::SIZE && y <= Chunk::HEIGHT && y >= 0 && z >= 0 && z < Chunk::SIZE)
    {
        if (Block::transparent(chunk.GetBlockID(glm::ivec3(x,y,z))) && isSolid)
        {
            return true;
        }
        else if(Block::transparent(chunk.GetBlockID(glm::ivec3(x,y,z))) && !isSolid && chunk.GetBlockID(glm::ivec3(x,y,z)) == 0) //empty
        {
            return true;
        }
    }
    return false;
}

void ChunkMeshGeneration::GenFaces(Chunk& chunk)
{
    int numFaces = 0;
    int numTransparentFaces = 0;
    for (int x = 0; x < Chunk::SIZE; x++)
    {
        for (int z = 0; z < Chunk::SIZE; z++)
        {
            for (int y = 0; y < Chunk::HEIGHT; y++)
            {
                glm::vec3 blockWorldPos = glm::vec3(x + chunk.chunkPosition.x * Chunk::SIZE, y, z + chunk.chunkPosition.y * Chunk::SIZE);
                if (!Block::transparent(chunk.GetBlockID(glm::ivec3(x,y,z))))
                {
                    AddFaces(x,y,z, numFaces, true, chunk);
                }
                else if(chunk.GetBlockID(glm::ivec3(x,y,z)) != 0) //if not empty
                {
                    //add to transparent mesh, integrate face only if bordering empty
                    AddFaces(x,y,z, numTransparentFaces, false, chunk);
                }
            }
        }
    }
    AddIndices(numFaces, chunk.chunkData.chunkIndices, chunk.chunkData.indexCount);
    AddIndices(numTransparentFaces, chunk.chunkData.transparentIndices, chunk.chunkData.transparentIndexCount);
}
void ChunkMeshGeneration::AddFaces(int x, int y, int z, int &numFaces, bool isSolid, Chunk& chunk) //checks the isSolid faces and adds them
{
    BlockType type = BlockIDMap[chunk.GetBlockID(glm::ivec3(x,y,z))];

    glm::vec3 blockWorldPos = glm::vec3(x + chunk.chunkPosition.x * Chunk::SIZE, y, z + chunk.chunkPosition.y * Chunk::SIZE);

    int leftXoffset = x - 1;

    int rightXoffset = x + 1;

    int frontZoffset = z + 1;

    int backZoffset = z - 1;

    int topYoffset = y + 1;

    int bottomYoffset = y - 1;

    if (CheckFace(leftXoffset, y, z, isSolid, chunk))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::LEFT, type, blockWorldPos), isSolid, chunk);
        numFaces++;
    }
    if (CheckFace(rightXoffset, y, z, isSolid, chunk))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::RIGHT, type, blockWorldPos), isSolid, chunk);
        numFaces++;
    }
    if (CheckFace(x, y, frontZoffset, isSolid, chunk))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::FRONT, type, blockWorldPos), isSolid, chunk);
        numFaces++;
    }
    if (CheckFace(x, y, backZoffset, isSolid, chunk))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::BACK, type, blockWorldPos), isSolid, chunk);
        numFaces++;
    }
    if (CheckFace(x, topYoffset, z, isSolid, chunk))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::TOP, type, blockWorldPos), isSolid, chunk);
        numFaces++;
    }
    if (CheckFace(x, bottomYoffset, z, isSolid, chunk))
    {
        IntegrateFace(Block::GetFace(CraftMine::Faces::BOTTOM, type, blockWorldPos), isSolid, chunk);
        numFaces++;
    }
}

void ChunkMeshGeneration::UpdateSide(CraftMine::Faces face, Chunk& chunk)
{
    Chunk* tempChunk = nullptr;
    int numFaces = 0;
    int numTransparentFaces = 0;

    //Used for neighbouring blocks, we loop through the edges of chunks and compare these two
    //e.g. our block could be in the start edge meaning neighbouring one will be in the endEdge

    int startEdge = 0;
    int endEdge = Chunk::SIZE - 1;

    switch (face)
    {
        case CraftMine::LEFT:
            tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x - 1, chunk.chunkPosition.y);
            for (int y = 0; y < Chunk::HEIGHT; y++)
            {
                for (int z = 0; z < Chunk::SIZE; z++)
                {
                    chunk.chunkBools.leftSideUpdated = true;
                    AddEdgeFaces(glm::ivec3(startEdge, y, z), numFaces, numTransparentFaces, z, endEdge, tempChunk, CraftMine::LEFT, chunk);
                }
            }
            break;
        case CraftMine::RIGHT:
            tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x + 1, chunk.chunkPosition.y);
            for (int y = 0; y < Chunk::HEIGHT; y++)
            {
                for (int z = 0; z < Chunk::SIZE; z++)
                {
                    chunk.chunkBools.rightSideUpdated = true;
                    AddEdgeFaces(glm::ivec3(endEdge, y, z), numFaces, numTransparentFaces, z, startEdge, tempChunk, CraftMine::RIGHT, chunk);
                }
            }
            break;
        case CraftMine::FRONT:
            tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x, chunk.chunkPosition.y + 1);
            for (int x = 0; x < Chunk::SIZE; x++)
            {
                for (int y = 0; y < Chunk::HEIGHT; y++)
                {
                    chunk.chunkBools.frontUpdated = true;
                    AddEdgeFaces(glm::ivec3(x, y, endEdge), numFaces, numTransparentFaces, startEdge, x, tempChunk, CraftMine::FRONT, chunk);
                }
            }
            break;
        case CraftMine::BACK:
            tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x, chunk.chunkPosition.y - 1);
            for (int x = 0; x < Chunk::SIZE; x++)
            {
                for (int y = 0; y < Chunk::HEIGHT; y++)
                {
                    chunk.chunkBools.backUpdated = true;
                    AddEdgeFaces(glm::ivec3(x, y, startEdge), numFaces, numTransparentFaces, endEdge, x, tempChunk, CraftMine::BACK, chunk);
                }
            }
            break;
    }
    AddIndices(numFaces, chunk.chunkData.chunkIndices, chunk.chunkData.indexCount);
    AddIndices(numTransparentFaces, chunk.chunkData.transparentIndices, chunk.chunkData.transparentIndexCount);
}
void ChunkMeshGeneration::AddEdgeFaces(glm::ivec3 localBlockPos, int &numFaces, int &numTransparentFaces, int neighbourZ, int neighbourX, Chunk* tempChunk, Faces face, Chunk& chunk) //used to add the faces neighbouring chunks
{
    glm::vec3 blockWorldPos = glm::vec3(localBlockPos.x + chunk.chunkPosition.x * Chunk::SIZE, localBlockPos.y, localBlockPos.z + chunk.chunkPosition.y * Chunk::SIZE);

    unsigned char blockID = chunk.GetBlockID(localBlockPos);

    unsigned char neighbourBlockID = tempChunk->GetBlockID(glm::ivec3(neighbourX, localBlockPos.y, neighbourZ));

    BlockType type = BlockIDMap[blockID];

    if (!Block::transparent(blockID) && Block::transparent(neighbourBlockID))
    {
        IntegrateFace(Block::GetFace(face, type, blockWorldPos), true, chunk);
        numFaces++;
    }
    else if(Block::transparent(blockID) && blockID != 0 && neighbourBlockID == 0)
    {
        IntegrateFace(Block::GetFace(face, type, blockWorldPos), false, chunk);
        numTransparentFaces++;
    }
}

void ChunkMeshGeneration::UpdateNeighbours(Chunk& chunk)
{
    //update the right side of the left chunk
    if (chunk.chunkPosition.x > 0)
    {
        Chunk* tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x - 1, chunk.chunkPosition.y);

        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData)
        {
            UpdateSide(CraftMine::LEFT, chunk);
            if(!tempChunk->chunkBools.rightSideUpdated)
            {
                UpdateSide(CraftMine::RIGHT, *tempChunk);
                tempChunk->generatedBuffData = false;
                chunk.world.loadedChunks.push(tempChunk);
            }
        }
        else if (tempChunk != nullptr && tempChunk->generatedBlockData)
        {
            UpdateSide(CraftMine::LEFT, chunk);
        }
    }
    //Update the left side of the right chunk
    if (chunk.chunkPosition.x < World::SIZE - 1)
    {
        Chunk* tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x + 1, chunk.chunkPosition.y);
        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData)
        {
            UpdateSide(CraftMine::RIGHT, chunk);
            if(!tempChunk->chunkBools.leftSideUpdated)
            {
                UpdateSide(CraftMine::LEFT, *tempChunk);
                tempChunk->generatedBuffData = false;
                chunk.world.loadedChunks.push(tempChunk);
            }
        }
        else if (tempChunk != nullptr && tempChunk->generatedBlockData)
        {
            UpdateSide(CraftMine::RIGHT, chunk);
        }
    }
    //Update back side of the front chunk
    if (chunk.chunkPosition.y < World::SIZE - 1)
    {
        Chunk* tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x, chunk.chunkPosition.y + 1);
        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData)
        {
            UpdateSide(CraftMine::FRONT, chunk);
            if(!tempChunk->chunkBools.backUpdated)
            {
                UpdateSide(CraftMine::BACK, *tempChunk);
                tempChunk->generatedBuffData = false;
                chunk.world.loadedChunks.push(tempChunk);
            }
        }
        else if (tempChunk != nullptr && tempChunk->generatedBlockData)
        {
            UpdateSide(CraftMine::FRONT, chunk);
        }

    }
    //Update front side of the back chunk
    if (chunk.chunkPosition.y > 0)
    {
        Chunk* tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x, chunk.chunkPosition.y - 1);
        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData)
        {
            UpdateSide(CraftMine::BACK, chunk);
            if(!tempChunk->chunkBools.frontUpdated)
            {
                UpdateSide(CraftMine::FRONT, *tempChunk);
                tempChunk->generatedBuffData = false;
                chunk.world.loadedChunks.push(tempChunk);
            }
        }
        else if (tempChunk != nullptr && tempChunk->generatedBlockData)
        {
            UpdateSide(CraftMine::BACK, chunk);
        }
    }
}
void ChunkMeshGeneration::IntegrateFace(FaceData faceData, bool solid, Chunk& chunk)
{
    //FaceData faceData = block.GetFace(face);
    if(solid) {
        chunk.chunkData.chunkVerts.insert(chunk.chunkData.chunkVerts.end(), faceData.vertices.begin(), faceData.vertices.end());
        chunk.chunkData.chunkUVs.insert(chunk.chunkData.chunkUVs.end(), faceData.texCoords.begin(), faceData.texCoords.end());
        //each vert has 4 points so need to repeat 4 times, can be adjusted to make nicer lighting later
        for(int i = 0; i < 4; i++)
        {
            chunk.chunkData.chunkBrightnessFloats.push_back(faceData.brightness);
        }
    }
    else
    {
        chunk.chunkData.transparentVerts.insert(chunk.chunkData.transparentVerts.end(), faceData.vertices.begin(), faceData.vertices.end());
        chunk.chunkData.transparentUVs.insert(chunk.chunkData.transparentUVs.end(), faceData.texCoords.begin(), faceData.texCoords.end());
        for(int i = 0; i < 4; i++)
        {
            chunk.chunkData.transparentBrightnessFloats.push_back(faceData.brightness);
        }

    }
}
void ChunkMeshGeneration::AddIndices(int amtFaces, std::vector<GLuint> &indices, GLsizei &_indexCount)
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