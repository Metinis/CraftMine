#include "ChunkMeshGeneration.h"

bool ChunkMeshGeneration::CheckFace(const int x, const int y, const int z, const bool isSolid, unsigned char originalID, const Chunk &chunk) {
    if (x >= 0 && x < Chunk::SIZE && y <= Chunk::HEIGHT && y >= 0 && z >= 0 && z < Chunk::SIZE) {
        if (Block::hasCustomMesh(chunk.GetBlockID(glm::ivec3(x, y, z)))) {
            return true;
        }
        if (Block::isTransparent(chunk.GetBlockID(glm::ivec3(x, y, z))) && isSolid) {
            return true;
        }
        if (Block::isTransparent(chunk.GetBlockID(glm::ivec3(x, y, z))) && !isSolid && chunk.
                   GetBlockID(glm::ivec3(x, y, z)) == 0) //empty
        {
            return true;
        }
    }

    return false;
}

void ChunkMeshGeneration::GenFaces(Chunk &chunk) {
    int numFaces = 0;
    int numTransparentFaces = 0;
    for (int x = 0; x < Chunk::SIZE; x++) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            for (int y = 0; y < Chunk::HEIGHT; y++) {
                const unsigned char blockID = chunk.GetBlockID(glm::ivec3(x,y,z));
                if (chunk.GetBlockID(glm::ivec3(x, y, z)) != 0) {
                    if (Block::isSolid(blockID) || Block::hasCustomMesh(blockID)) {
                        AddFaces(x, y, z, numFaces, true, chunk);
                    } else if (!Block::isSolid(blockID)) {
                        AddFaces(x, y, z, numTransparentFaces, false, chunk);
                    }
                }
            }
        }
    }
    AddIndices(numFaces, chunk.chunkData.chunkIndices, chunk.chunkData.indexCount);
    AddIndices(numTransparentFaces, chunk.chunkData.nonSolidIndices, chunk.chunkData.nonSolidIndexCount);
}

void ChunkMeshGeneration::AddFaces(int x, int y, int z, int &numFaces, bool isSolid, Chunk &chunk)
//checks the isSolid faces and adds them
{
    const unsigned char id = chunk.GetBlockID(glm::ivec3(x, y, z));
    const BlockType type = BlockIDMap[id];

    const glm::vec3 blockWorldPos = glm::ivec3(x + chunk.chunkPosition.x * Chunk::SIZE, y,
                                        z + chunk.chunkPosition.y * Chunk::SIZE);

    const glm::vec4 rgbiLight = chunk.getBlockLightNormalised(glm::ivec3(x,y,z));

    if (Block::hasCustomMesh(id)) {
        IntegrateFace(Block::GetFace(CraftMine::Faces::FRONT, type, blockWorldPos), rgbiLight, isSolid, chunk);

        IntegrateFace(Block::GetFace(CraftMine::Faces::BACK, type, blockWorldPos), rgbiLight, isSolid, chunk);

        IntegrateFace(Block::GetFace(CraftMine::Faces::RIGHT, type, blockWorldPos), rgbiLight, isSolid, chunk);

        IntegrateFace(Block::GetFace(CraftMine::Faces::LEFT, type, blockWorldPos), rgbiLight, isSolid, chunk);
        numFaces += 4;
        return;
    }

    const int leftXoffset = x - 1;

    const int rightXoffset = x + 1;

    const int frontZoffset = z + 1;

    const int backZoffset = z - 1;

    const int topYoffset = y + 1;

    const int bottomYoffset = y - 1;

    if (CheckFace(leftXoffset, y, z, isSolid, id, chunk)) {
        IntegrateFace(Block::GetFace(CraftMine::Faces::LEFT, type, blockWorldPos), rgbiLight, isSolid, chunk);
        numFaces++;
    }
    if (CheckFace(rightXoffset, y, z, isSolid, id, chunk)) {
        IntegrateFace(Block::GetFace(CraftMine::Faces::RIGHT, type, blockWorldPos), rgbiLight, isSolid, chunk);
        numFaces++;
    }
    if (CheckFace(x, y, frontZoffset, isSolid, id, chunk)) {
        IntegrateFace(Block::GetFace(CraftMine::Faces::FRONT, type, blockWorldPos), rgbiLight, isSolid, chunk);
        numFaces++;
    }
    if (CheckFace(x, y, backZoffset, isSolid, id, chunk)) {
        IntegrateFace(Block::GetFace(CraftMine::Faces::BACK, type, blockWorldPos), rgbiLight, isSolid, chunk);
        numFaces++;
    }
    //Seperate check for liquid tops
    if ((CheckFace(x, topYoffset, z, isSolid, id, chunk)) || (!isSolid && Block::isSolid(chunk.GetBlockID(glm::ivec3(x, topYoffset, z))))) {
        IntegrateFace(Block::GetFace(CraftMine::Faces::TOP, type, blockWorldPos), rgbiLight, isSolid, chunk);
        numFaces++;
    }
    if (CheckFace(x, bottomYoffset, z, isSolid, id, chunk)) {
        IntegrateFace(Block::GetFace(CraftMine::Faces::BOTTOM, type, blockWorldPos), rgbiLight, isSolid, chunk);
        numFaces++;
    }
}

void ChunkMeshGeneration::UpdateSide(CraftMine::Faces face, Chunk &chunk) {
    Chunk *tempChunk;
    int numFaces = 0;
    int numTransparentFaces = 0;

    //Used for neighbouring blocks, we loop through the edges of chunks and compare these two
    //e.g. our block could be in the start edge meaning neighbouring one will be in the endEdge

    int startEdge = 0;
    int endEdge = Chunk::SIZE - 1;

    switch (face) {
        case CraftMine::LEFT:
            tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x - 1, chunk.chunkPosition.y);
            if (tempChunk != nullptr) {
                for (int y = 0; y < Chunk::HEIGHT; y++) {
                    for (int z = 0; z < Chunk::SIZE; z++) {
                        chunk.chunkBools.leftSideUpdated = true;
                        AddEdgeFaces(glm::ivec3(startEdge, y, z), numFaces, numTransparentFaces, z, endEdge, tempChunk,
                                     CraftMine::LEFT, chunk);
                    }
                }
            }
            break;
        case CraftMine::RIGHT:
            tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x + 1, chunk.chunkPosition.y);
            if (tempChunk != nullptr) {
                for (int y = 0; y < Chunk::HEIGHT; y++) {
                    for (int z = 0; z < Chunk::SIZE; z++) {
                        chunk.chunkBools.rightSideUpdated = true;
                        AddEdgeFaces(glm::ivec3(endEdge, y, z), numFaces, numTransparentFaces, z, startEdge, tempChunk,
                                     CraftMine::RIGHT, chunk);
                    }
                }
            }

            break;
        case CraftMine::FRONT:
            tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x, chunk.chunkPosition.y + 1);
            if (tempChunk != nullptr) {
                for (int x = 0; x < Chunk::SIZE; x++) {
                    for (int y = 0; y < Chunk::HEIGHT; y++) {
                        chunk.chunkBools.frontUpdated = true;
                        AddEdgeFaces(glm::ivec3(x, y, endEdge), numFaces, numTransparentFaces, startEdge, x, tempChunk,
                                     CraftMine::FRONT, chunk);
                    }
                }
            }

            break;
        case CraftMine::BACK:
            tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x, chunk.chunkPosition.y - 1);
            if (tempChunk != nullptr) {
                for (int x = 0; x < Chunk::SIZE; x++) {
                    for (int y = 0; y < Chunk::HEIGHT; y++) {
                        chunk.chunkBools.backUpdated = true;
                        AddEdgeFaces(glm::ivec3(x, y, startEdge), numFaces, numTransparentFaces, endEdge, x, tempChunk,
                                     CraftMine::BACK, chunk);
                    }
                }
            }
        default:
            break;

    }
    AddIndices(numFaces, chunk.chunkData.chunkIndices, chunk.chunkData.indexCount);
    AddIndices(numTransparentFaces, chunk.chunkData.nonSolidIndices, chunk.chunkData.nonSolidIndexCount);
}

void ChunkMeshGeneration::AddEdgeFaces(const glm::ivec3 localBlockPos, int &numFaces, int &numNonSolidFaces, const int neighbourZ,
                                       const int neighbourX, const Chunk *tempChunk, const Faces face,
                                       Chunk &chunk) //used to add the faces neighbouring chunks
{
    const auto blockWorldPos = glm::vec3(localBlockPos.x + chunk.chunkPosition.x * Chunk::SIZE, localBlockPos.y,
                                        localBlockPos.z + chunk.chunkPosition.y * Chunk::SIZE);

    const unsigned char blockID = chunk.GetBlockID(localBlockPos);

    const unsigned char neighbourBlockID = tempChunk->GetBlockID(glm::ivec3(neighbourX, localBlockPos.y, neighbourZ));

    const glm::vec4 rgbiLight = chunk.getBlockLightNormalised(localBlockPos);

    if (Block::hasCustomMesh(blockID) || blockID == 0)
        return;

    const BlockType type = BlockIDMap[blockID];
    if ((Block::isSolid(blockID) && !Block::isSolid(neighbourBlockID)) ||
        (Block::isSolid(blockID) && neighbourBlockID == 0) ||
        (Block::isSolid(blockID) && !Block::isTransparent(blockID) && Block::isTransparent(neighbourBlockID)) ||
        (Block::isSolid(blockID) && Block::isTransparent(blockID) && !Block::isTransparent(neighbourBlockID)))
    {
        IntegrateFace(Block::GetFace(face, type, blockWorldPos), rgbiLight, true, chunk);
        numFaces++;
    }
    else if (!Block::isSolid(blockID) && neighbourBlockID == 0) {
        IntegrateFace(Block::GetFace(face, type, blockWorldPos), rgbiLight, false, chunk);
        numFaces++;
    }
}

void ChunkMeshGeneration::UpdateNeighbours(Chunk &chunk) {
    //update the right side of the left chunk
    if (&chunk == nullptr) {
        return;
    }
    if (chunk.chunkPosition.x > 0) {
        Chunk *tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x - 1, chunk.chunkPosition.y);

        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData && !tempChunk->
            inThread) {
            UpdateSide(CraftMine::LEFT, chunk);
            if (!tempChunk->chunkBools.rightSideUpdated) {
                tempChunk->inThread = true;
                tempChunk->generatedBuffData = false;
                UpdateSide(CraftMine::RIGHT, *tempChunk);
                tempChunk->inThread = false;

                WorldThreading::addToLoadedChunks(tempChunk);
            }
        } else if (tempChunk != nullptr && tempChunk->generatedBlockData) {
            UpdateSide(CraftMine::LEFT, chunk);
        }
    } else {
        chunk.chunkBools.leftSideUpdated = true;
    }
    //Update the left side of the right chunk
    if (chunk.chunkPosition.x < World::SIZE - 1) {
        Chunk *tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x + 1, chunk.chunkPosition.y);
        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData && !tempChunk->
            inThread) {
            UpdateSide(CraftMine::RIGHT, chunk);
            if (!tempChunk->chunkBools.leftSideUpdated) {
                tempChunk->inThread = true;
                tempChunk->generatedBuffData = false;
                UpdateSide(CraftMine::LEFT, *tempChunk);
                tempChunk->inThread = false;

                WorldThreading::addToLoadedChunks(tempChunk);
            }
        } else if (tempChunk != nullptr && tempChunk->generatedBlockData) {
            UpdateSide(CraftMine::RIGHT, chunk);
        }
    } else {
        chunk.chunkBools.rightSideUpdated = true;
    }
    //Update back side of the front chunk
    if (chunk.chunkPosition.y < World::SIZE - 1) {
        Chunk *tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x, chunk.chunkPosition.y + 1);
        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData && !tempChunk->
            inThread) {
            UpdateSide(CraftMine::FRONT, chunk);
            if (!tempChunk->chunkBools.backUpdated) {
                tempChunk->inThread = true;
                tempChunk->generatedBuffData = false;
                UpdateSide(CraftMine::BACK, *tempChunk);
                tempChunk->inThread = false;
                WorldThreading::addToLoadedChunks(tempChunk);
            }
        } else if (tempChunk != nullptr && tempChunk->generatedBlockData) {
            UpdateSide(CraftMine::FRONT, chunk);
        }
    } else {
        chunk.chunkBools.frontUpdated = true;
    }
    //Update front side of the back chunk
    if (chunk.chunkPosition.y > 0) {
        Chunk *tempChunk = chunk.world.GetChunk(chunk.chunkPosition.x, chunk.chunkPosition.y - 1);
        if (tempChunk != nullptr && tempChunk->generatedBlockData && tempChunk->generatedBuffData && !tempChunk->
            inThread) {
            UpdateSide(CraftMine::BACK, chunk);
            if (!tempChunk->chunkBools.frontUpdated) {
                tempChunk->inThread = true;
                tempChunk->generatedBuffData = false;
                UpdateSide(CraftMine::FRONT, *tempChunk);
                tempChunk->inThread = false;
                WorldThreading::addToLoadedChunks(tempChunk);
            }
        } else if (tempChunk != nullptr && tempChunk->generatedBlockData) {
            UpdateSide(CraftMine::BACK, chunk);
        }
    } else {
        chunk.chunkBools.backUpdated = true;
    }
}
void ChunkMeshGeneration::IntegrateFace(FaceData faceData, const glm::vec4 rgbiLight, const bool isSolid, Chunk &chunk) {
    if (isSolid) {
        chunk.chunkData.chunkVerts.insert(chunk.chunkData.chunkVerts.end(), faceData.vertices.begin(),
                                          faceData.vertices.end());
        chunk.chunkData.chunkUVs.insert(chunk.chunkData.chunkUVs.end(), faceData.texCoords.begin(),
                                        faceData.texCoords.end());
        chunk.chunkData.chunkNormals.insert(chunk.chunkData.chunkNormals.end(), faceData.normals.begin(),
                                            faceData.normals.end());
        //each vert has 4 points so need to repeat 4 times, can be adjusted to make nicer lighting later
        for (int i = 0; i < 4; i++) {
            chunk.chunkData.chunkRGBIValues.push_back(faceData.brightness * rgbiLight);
        }
    } else if (&chunk != nullptr) {
        chunk.chunkData.nonSolidVerts.insert(chunk.chunkData.nonSolidVerts.end(), faceData.vertices.begin(),
                                             faceData.vertices.end());
        chunk.chunkData.nonSolidUVs.insert(chunk.chunkData.nonSolidUVs.end(), faceData.texCoords.begin(),
                                           faceData.texCoords.end());
        chunk.chunkData.nonSolidNormals.insert(chunk.chunkData.nonSolidNormals.end(), faceData.normals.begin(),
                                               faceData.normals.end());
        for (int i = 0; i < 4; i++) {
            chunk.chunkData.nonSolidRGBIValues.push_back(faceData.brightness * rgbiLight);
        }
    }
}

void ChunkMeshGeneration::IntegrateFace(FaceData faceData, const bool isSolid, Chunk &chunk) {
    if (isSolid) {
        chunk.chunkData.chunkVerts.insert(chunk.chunkData.chunkVerts.end(), faceData.vertices.begin(),
                                          faceData.vertices.end());
        chunk.chunkData.chunkUVs.insert(chunk.chunkData.chunkUVs.end(), faceData.texCoords.begin(),
                                        faceData.texCoords.end());
        chunk.chunkData.chunkNormals.insert(chunk.chunkData.chunkNormals.end(), faceData.normals.begin(),
                                            faceData.normals.end());
        //each vert has 4 points so need to repeat 4 times, can be adjusted to make nicer lighting later
        for (int i = 0; i < 4; i++) {
            chunk.chunkData.chunkRGBIValues.push_back(faceData.brightness * glm::vec4(1.0f,1.0f,1.0f, 1.0f));
        }
    } else if (&chunk != nullptr) {
        chunk.chunkData.nonSolidVerts.insert(chunk.chunkData.nonSolidVerts.end(), faceData.vertices.begin(),
                                             faceData.vertices.end());
        chunk.chunkData.nonSolidUVs.insert(chunk.chunkData.nonSolidUVs.end(), faceData.texCoords.begin(),
                                           faceData.texCoords.end());
        chunk.chunkData.nonSolidNormals.insert(chunk.chunkData.nonSolidNormals.end(), faceData.normals.begin(),
                                               faceData.normals.end());
        for (int i = 0; i < 4; i++) {
            chunk.chunkData.nonSolidRGBIValues.push_back(faceData.brightness * glm::vec4(1.0f,1.0f,1.0f, 1.0f));
        }
    }
}

void ChunkMeshGeneration::AddIndices(int amtFaces, std::vector<GLuint> &indices, GLsizei &_indexCount) {
    for (int i = 0; i < amtFaces; i++) {
        indices.push_back(0 + _indexCount);
        indices.push_back(1 + _indexCount);
        indices.push_back(2 + _indexCount);
        indices.push_back(2 + _indexCount);
        indices.push_back(3 + _indexCount);
        indices.push_back(0 + _indexCount);

        _indexCount += 4;
        //uses element index, since each face only has 4 indices, we increment this everytime any block face is added
    }
}
