//
// Created by vodkasas on 03/02/24.
//

#include "ChunkGeneration.h"

void ChunkGeneration::GenBlocks(Chunk &chunk) {
    float heightMap[Chunk::HEIGHT * Chunk::HEIGHT];

    GenChunk(heightMap, chunk);

    FastNoise noise3D;
    noise3D.SetNoiseType(FastNoise::PerlinFractal);
    noise3D.SetSeed(123);
    noise3D.SetFrequency(0.045f);

    int seaLevel = Chunk::HEIGHT / 2;

    for (int y = Chunk::HEIGHT - 1; y >= 0; y--) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            for (int x = 0; x < Chunk::SIZE; x++) {
                float worldX = static_cast<float>(x) + (Chunk::SIZE * static_cast<float>(chunk.chunkPosition.x));
                float worldZ = static_cast<float>(z) + (Chunk::SIZE * static_cast<float>(chunk.chunkPosition.y));
                float worldY = static_cast<float>(y);
                float noiseValue = noise3D.GetNoise(worldX, worldY, worldZ);

                // Carve out caves based on noise value threshold

                int columnHeight = (int) heightMap[x + Chunk::SIZE * z] + (seaLevel - 30);
                //add sealevel so noise generates above it

                unsigned char id = 0;
                if (chunk.GetBlockID(glm::ivec3(x, y, z)) == 0) {
                    //so that trees/structures don't get overwritten

                    if (y >= columnHeight && y + 2 < seaLevel) {
                        //last argument is the sea level

                        id = 5;
                    } else if (noiseValue > 0.3f && y > 0) {
                        if (y < Chunk::HEIGHT - 1 && chunk.GetBlockID(glm::ivec3(x, y + 1, z)) == 5) {
                            //TODO check horizontal blocks for adding water, including neighbouring chunks
                            id = 5;
                            //UpdateWater(chunk, glm::ivec3(x,y,z));
                        } else {
                            id = 0;
                        }
                    } else if (y <= columnHeight - 1 && y >= columnHeight - 3 && y < seaLevel - 2) {
                        id = 6; //sand
                    } else if (y == columnHeight - 1) {
                        id = 1; //Refer to id map, grass layer
                        //check to generate tree
                        if (shouldGenTree()) {
                            id = 2; //if there's tree above, make dirt block, not grass
                            genTree(glm::vec3(x, y + 1, z), chunk);
                        } else if (shouldGenFlower()) {
                            std::mt19937 rng(std::random_device{}());
                            // Define the probability (1 in 250 chance)
                            std::uniform_int_distribution<> dis(1, 2);
                            if (dis(rng) == 1) {
                                chunk.SetBlock(glm::ivec3(x, y + 1, z), 70);
                            } else {
                                chunk.SetBlock(glm::ivec3(x, y + 1, z), 71);
                            }
                        }
                    } else if (y > columnHeight - 4 && y < columnHeight) {
                        if (y < Chunk::HEIGHT - 1 && chunk.GetBlockID(glm::ivec3(x, y + 1, z)) == 0) {
                            id = 1;
                        } else {
                            id = 2; //dirt layer
                        }
                    } else if (y <= columnHeight - 4 && y > 0) {
                        id = 3; //stone layer
                    } else if (y == 0) {
                        id = 4; //bedrock layer
                    }
                    chunk.SetBlock(glm::ivec3(x, y, z), id);
                }
            }
        }
    }
}


bool ChunkGeneration::shouldGenTree() {
    std::mt19937 rng(std::random_device{}());
    // Define the probability (1 in 250 chance)
    std::uniform_int_distribution<> dis(1, 250);

    // Generate a random number
    int randomNumber = dis(rng);

    // Check if the random number falls within the desired range
    return randomNumber == 1;
}

bool ChunkGeneration::shouldGenFlower() {
    std::mt19937 rng(std::random_device{}());
    // Define the probability (1 in 250 chance)
    std::uniform_int_distribution<> dis(1, 100);

    // Generate a random number
    int randomNumber = dis(rng);

    // Check if the random number falls within the desired range
    return randomNumber == 1;
}

void ChunkGeneration::genTree(glm::ivec3 treeCoord, Chunk &chunk) //starts from bottom block
{
    std::random_device rd;
    std::mt19937 rng(rd());

    // Define the range
    int lower_bound = 4;
    int upper_bound = 5;

    // Create a uniform integer distribution
    std::uniform_int_distribution<int> distribution(lower_bound, upper_bound);

    // Generate a random number between 5 and 7
    int treeHeight = distribution(rng);
    int leafHeight = 2;

    for (int y = treeCoord.y; y < (treeHeight + treeCoord.y + leafHeight) && y < Chunk::HEIGHT; y++) {
        int localY = y - treeCoord.y;
        //set the tree
        if (y < treeHeight + treeCoord.y)
            chunk.SetBlock(glm::ivec3(treeCoord.x, y, treeCoord.z), 7); //7 is wood

        if (localY >= treeHeight - 2 && localY < treeHeight) {
            //bottom layer
            int startX = treeCoord.x - 2;
            int endX = treeCoord.x + 3;
            int startZ = treeCoord.z - 2;
            int endZ = treeCoord.z + 3;

            generateLeaves(startX, endX, startZ, endZ, y, chunk);
        }
        //top layer
        else if (localY >= treeHeight && localY < (treeHeight + leafHeight)) {
            int startX = treeCoord.x - 1;
            int endX = treeCoord.x + 2;
            int startZ = treeCoord.z - 1;
            int endZ = treeCoord.z + 2;

            generateLeaves(startX, endX, startZ, endZ, y, chunk);
        }
    }
}

void ChunkGeneration::generateLeaves(int startX, int endX, int startZ, int endZ, int y, Chunk &chunk) {
    for (int x = startX; x < endX; x++) {
        for (int z = startZ; z < endZ; z++) {
            //if x and z are in this chunk
            if (x >= 0 && x < Chunk::SIZE && z >= 0 && z < Chunk::SIZE) {
                if (chunk.GetBlockID(glm::ivec3(x, y, z)) == 0)
                    chunk.SetBlock(glm::ivec3(x, y, z), 8);
            } else {
                //find the chunk x and z are in, check if it is not null, convert x and y to that local chunk coordinates and set blocks and update buffers
                //Add to loaded chunks for updating

                int tempChunkX = (x < 0)
                                     ? (chunk.chunkPosition.x - 1)
                                     : (x > Chunk::SIZE - 1)
                                           ? (chunk.chunkPosition.x + 1)
                                           : chunk.chunkPosition.x;

                int tempChunkZ = (z < 0)
                                     ? (chunk.chunkPosition.y - 1)
                                     : (z > Chunk::SIZE - 1)
                                           ? (chunk.chunkPosition.y + 1)
                                           : chunk.chunkPosition.y;

                if (tempChunkX >= 0 && tempChunkX < World::SIZE && tempChunkZ >= 0 && tempChunkZ < World::SIZE) {
                    Chunk &tempChunk = *chunk.world.GetChunk(tempChunkX, tempChunkZ);

                    int tempChunkLocalX = (x < 0) ? (Chunk::SIZE + x) : (x > Chunk::SIZE - 1) ? (x - Chunk::SIZE) : x;

                    int tempChunkLocalZ = (z < 0) ? (Chunk::SIZE + z) : (z > Chunk::SIZE - 1) ? (z - Chunk::SIZE) : z;

                    if (&tempChunk != nullptr && tempChunk.generatedBlockData && !tempChunk.inThread && tempChunk.
                        generatedBuffData) {
                        if (tempChunk.GetBlockID(glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ)) == 0) {
                            tempChunk.SetBlock(glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ), 8);
                            chunk.world.chunksToLoadData.push_back(tempChunk.chunkPosition);
                        }
                    } else if (&tempChunk != nullptr && tempChunk.generatedBlockData && !tempChunk.inThread && !
                               tempChunk.generatedBuffData) {
                        if (tempChunk.GetBlockID(glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ)) == 0) {
                            tempChunk.SetBlock(glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ), 8);
                        }
                    } else
                    // if(&tempChunk == nullptr || (&tempChunk != nullptr && (!tempChunk.inThread || (tempChunk.inThread && tempChunk.generatedBlockData))))
                    {
                        chunk.world.mutexBlocksToBeAddedList.lock();
                        chunk.world.blocksToBeAddedList.push_back(BlocksToBeAdded{
                            glm::ivec2(tempChunkX, tempChunkZ), glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ), 8
                        });
                        chunk.world.mutexBlocksToBeAddedList.unlock();
                    }
                }
            }
        }
    }
}

void ChunkGeneration::UpdateWater(Chunk &chunk, glm::ivec3 waterPos) {
    int left = waterPos.x - 1;
    int right = waterPos.x + 1;
    int front = waterPos.z - 1;
    int back = waterPos.z + 1;
    int down = waterPos.y - 1;

    if (left >= 0) {
        if (chunk.GetBlockID(glm::ivec3(left, waterPos.y, waterPos.z)) == 0) {
            BlocksToBeAdded block = BlocksToBeAdded(chunk.chunkPosition, glm::ivec3(left, waterPos.y, waterPos.z), 5);
            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    } else {
        glm::ivec3 blockPosInBorderingChunk = glm::ivec3(Chunk::SIZE - 1, waterPos.y, waterPos.z);
        glm::ivec2 borderingChunkPosition = glm::ivec2(chunk.chunkPosition.x - 1, chunk.chunkPosition.y);

        if (chunk.world.GetChunk(borderingChunkPosition)->GetBlockID(blockPosInBorderingChunk) == 0) {

            BlocksToBeAdded block = BlocksToBeAdded(borderingChunkPosition, blockPosInBorderingChunk, 5);

            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    }
    if (right < Chunk::SIZE) {
        if (chunk.GetBlockID(glm::ivec3(right, waterPos.y, waterPos.z)) == 0) {
            BlocksToBeAdded block = BlocksToBeAdded(chunk.chunkPosition, glm::ivec3(right, waterPos.y, waterPos.z), 5);
            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    } else {
        glm::ivec3 blockPosInBorderingChunk = glm::ivec3(0, waterPos.y, waterPos.z);
        glm::ivec2 borderingChunkPosition = glm::ivec2(chunk.chunkPosition.x + 1, chunk.chunkPosition.y);

        if (chunk.world.GetChunk(borderingChunkPosition)->GetBlockID(blockPosInBorderingChunk) == 0) {

            BlocksToBeAdded block = BlocksToBeAdded(borderingChunkPosition, blockPosInBorderingChunk, 5);

            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    }
    if (front >= 0) {
        if (chunk.GetBlockID(glm::ivec3(waterPos.x, waterPos.y, front)) == 0) {
            BlocksToBeAdded block = BlocksToBeAdded(chunk.chunkPosition, glm::ivec3(waterPos.x, waterPos.y, front), 5);
            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    } else {
        glm::ivec3 blockPosInBorderingChunk = glm::ivec3(waterPos.x, waterPos.y, Chunk::SIZE - 1);
        glm::ivec2 borderingChunkPosition = glm::ivec2(chunk.chunkPosition.x, chunk.chunkPosition.y - 1);

        if (chunk.world.GetChunk(borderingChunkPosition)->GetBlockID(blockPosInBorderingChunk) == 0) {

            BlocksToBeAdded block = BlocksToBeAdded(borderingChunkPosition, blockPosInBorderingChunk, 5);

            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    }
    if (back < Chunk::SIZE) {
        if (chunk.GetBlockID(glm::ivec3(waterPos.x, waterPos.y, back)) == 0) {
            BlocksToBeAdded block = BlocksToBeAdded(chunk.chunkPosition, glm::ivec3(waterPos.x, waterPos.y, back), 5);
            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end()) {
                chunk.world.liquidToBeChecked.push_back(block);

                // std::cout<<"\ntest2";
            }
        }
    } else {
        glm::ivec3 blockPosInBorderingChunk = glm::ivec3(waterPos.x, waterPos.y, 0);
        glm::ivec2 borderingChunkPosition = glm::ivec2(chunk.chunkPosition.x, chunk.chunkPosition.y + 1);

        if (chunk.world.GetChunk(borderingChunkPosition)->GetBlockID(blockPosInBorderingChunk) == 0) {

            BlocksToBeAdded block = BlocksToBeAdded(borderingChunkPosition, blockPosInBorderingChunk, 5);

            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    }
    if (down >= 0) {
        if (chunk.GetBlockID(glm::ivec3(waterPos.x, down, waterPos.z)) == 0) {
            BlocksToBeAdded block = BlocksToBeAdded(chunk.chunkPosition, glm::ivec3(waterPos.x, down, waterPos.z), 5);
            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    }
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

void ChunkGeneration::GenChunk(float *heightMap, Chunk &chunk) {
    FastNoise noise;

    // Set noise parameters for more varied terrain
    noise.SetNoiseType(FastNoise::PerlinFractal); // Perlin noise for smooth terrain
    noise.SetFractalType(FastNoise::FBM); // FBM for multi-layered noise
    noise.SetFractalOctaves(16); // Increase octaves for more detail
    noise.SetSeed(123); // Example seed
    noise.SetFrequency(0.00835f); // Higher frequency for more hills/valleys
    float scaleFactor = 0.65f;
    /*FastNoise cliffNoise; // Additional noise for cliffs
    cliffNoise.SetNoiseType(FastNoise::PerlinFractal); // Use Perlin noise to create sharp cliffs
    cliffNoise.SetFractalType(FastNoise::FBM); // FBM for multi-layered noise
    cliffNoise.SetFractalOctaves(3); // Increase octaves for more detail
    cliffNoise.SetSeed(321); // Different seed for cliff variation
    cliffNoise.SetFrequency(0.002f); // Higher frequency for cliff-like formations*/


    for (int x = 0; x < Chunk::SIZE; x++) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            float worldX = static_cast<float>(x) + (Chunk::SIZE * static_cast<float>(chunk.chunkPosition.x));
            float worldZ = static_cast<float>(z) + (Chunk::SIZE * static_cast<float>(chunk.chunkPosition.y));

            float noiseValue = noise.GetNoise(worldX, worldZ);

            int terrainHeight = static_cast<int>((noiseValue + 1.0f) * 50.0f * scaleFactor);
            // Cliff variation from 3D noise
            /*float cliffVariation = cliffNoise.GetNoise(worldX, worldZ) * 100.0f; // Scale cliff influence
            if(cliffVariation > 0.5f) {
                terrainHeight+=cliffVariation;
            }*/

            heightMap[x + Chunk::SIZE * z] = terrainHeight;
        }
    }
}
