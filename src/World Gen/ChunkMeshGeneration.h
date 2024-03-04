//
// Created by vodkasas on 03/02/24.
//

#ifndef CRAFTMINE_CHUNKMESHGENERATION_H
#define CRAFTMINE_CHUNKMESHGENERATION_H
#pragma once
#include "Chunk.h"
#include "World.h"

class ChunkMeshGeneration {
private:
    static bool CheckFace(int x, int y, int z, bool isSolid, unsigned char originalID, Chunk& chunk);
    static void AddEdgeFaces(glm::ivec3 localBlockPos, int &numFaces, int &numTransparentFaces, int neighbourZ, int neighbourX, Chunk* tempChunk, Faces face, Chunk& chunk);
    static void AddFaces(int x, int y, int z, int &numFaces, bool isSolid, Chunk& chunk);
    static void IntegrateFace(FaceData faceData, bool isTransparent, Chunk& chunk);
    static void UpdateSide(CraftMine::Faces face, Chunk& chunk);
public:
    static void AddIndices(int amtFaces, std::vector<GLuint> &indices, GLsizei &_indexCount);
    static void GenFaces(Chunk& chunk);
    static void UpdateNeighbours(Chunk& chunk);

};


#endif //CRAFTMINE_CHUNKMESHGENERATION_H
