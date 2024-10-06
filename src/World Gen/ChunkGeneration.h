//
// Created by vodkasas on 03/02/24.
//

#ifndef CRAFTMINE_CHUNKGENERATION_H
#define CRAFTMINE_CHUNKGENERATION_H

#pragma once
#include "Chunk.h"
#include "World.h"

class ChunkGeneration {
private:
    static bool shouldGenTree();
    static void genTree(glm::ivec3 treeCoord, Chunk& chunk);
    static void generateLeaves(int startX, int endX, int startZ, int endZ, int y, Chunk& chunk);
    static void GenChunk(float* heightMap, Chunk& chunk);
public:
    static void GenBlocks(Chunk& chunk);

    static bool shouldGenFlower();

    static void UpdateWater(Chunk &chunk, glm::ivec3 waterPos);
};


#endif //CRAFTMINE_CHUNKGENERATION_H
