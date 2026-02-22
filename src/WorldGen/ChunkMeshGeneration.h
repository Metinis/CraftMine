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
    static bool checkFace(int x, int y, int z, bool isSolid, unsigned char originalID, const Chunk& chunk);
    static void addEdgeFaces(glm::ivec3 localBlockPos, int &numFaces, int &numNonSolidFaces, int neighbourZ, int neighbourX, const Chunk* tempChunk, Faces face, Chunk& chunk);
    static void addFaces(int x, int y, int z, int &numFaces, bool isSolid, Chunk& chunk);
    static void integrateFace(FaceData faceData, bool isSolid, Chunk& chunk);
    static void updateSide(CraftMine::Faces face, Chunk& chunk);
public:
    static void addIndices(int amtFaces, std::vector<GLuint> &indices, GLsizei &_indexCount);
    static void genFaces(Chunk& chunk);
    static void updateNeighbours(Chunk& chunk);

};


#endif //CRAFTMINE_CHUNKMESHGENERATION_H
