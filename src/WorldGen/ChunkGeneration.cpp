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

    constexpr int seaLevel = Chunk::HEIGHT / 2;

    for (int y = Chunk::HEIGHT - 1; y >= 0; y--) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            for (int x = 0; x < Chunk::SIZE; x++) {
                const float worldX = static_cast<float>(x) + (Chunk::SIZE * static_cast<float>(chunk.chunkPosition.x));
                const float worldZ = static_cast<float>(z) + (Chunk::SIZE * static_cast<float>(chunk.chunkPosition.y));
                const auto worldY = static_cast<float>(y);
                const float noiseValue = noise3D.GetNoise(worldX, worldY, worldZ);

                // Carve out caves based on noise value threshold

                const int columnHeight = static_cast<int>(heightMap[x + Chunk::SIZE * z] + (seaLevel - 30));
                //add sealevel so noise generates above it

                if (chunk.GetBlockID(glm::ivec3(x, y, z)) == 0) {
                    unsigned char id = 0;
                    //so that trees/structures don't get overwritten

                    if (y >= columnHeight && y + 2 < seaLevel) {
                        //last argument is the sea level

                        id = 5;
                    } else if (noiseValue > 0.3f && y > 0) {
                        if (y < Chunk::HEIGHT - 1 && ((chunk.GetBlockID(glm::ivec3(x, y + 1, z)) == 5) ||
                            (chunk.GetBlockID(glm::ivec3(x, y, z)) == 5) ||
                            (chunk.GetBlockID(glm::ivec3(x-1, y, z)) == 5) ||
                            (chunk.GetBlockID(glm::ivec3(x+1, y, z)) == 5) ||
                            (chunk.GetBlockID(glm::ivec3(x, y, z-1)) == 5) ||
                            (chunk.GetBlockID(glm::ivec3(x, y, z+1)) == 5))) {

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
    const int randomNumber = dis(rng);

    // Check if the random number falls within the desired range
    return randomNumber == 1;
}

void ChunkGeneration::genTree(glm::ivec3 treeCoord, Chunk &chunk) //starts from bottom block
{
    std::random_device rd;
    std::mt19937 rng(rd());

    // Define the range
    constexpr int lower_bound = 4;
    constexpr int upper_bound = 5;

    // Create a uniform integer distribution
    std::uniform_int_distribution<int> distribution(lower_bound, upper_bound);

    // Generate a random number between 5 and 7
    const int treeHeight = distribution(rng);
    constexpr int leafHeight = 2;

    for (int y = treeCoord.y; y < (treeHeight + treeCoord.y + leafHeight) && y < Chunk::HEIGHT; y++) {
        const int localY = y - treeCoord.y;
        //set the tree
        if (y < treeHeight + treeCoord.y)
            chunk.SetBlock(glm::ivec3(treeCoord.x, y, treeCoord.z), 7); //7 is wood

        if (localY >= treeHeight - 2 && localY < treeHeight) {
            //bottom layer
            const int startX = treeCoord.x - 2;
            const int endX = treeCoord.x + 3;
            const int startZ = treeCoord.z - 2;
            const int endZ = treeCoord.z + 3;

            generateLeaves(startX, endX, startZ, endZ, y, chunk);
        }
        //top layer
        else if (localY >= treeHeight && localY < (treeHeight + leafHeight)) {
            const int startX = treeCoord.x - 1;
            const int endX = treeCoord.x + 2;
            const int startZ = treeCoord.z - 1;
            const int endZ = treeCoord.z + 2;

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

                const int tempChunkX = (x < 0)
                                     ? (chunk.chunkPosition.x - 1)
                                     : (x > Chunk::SIZE - 1)
                                           ? (chunk.chunkPosition.x + 1)
                                           : chunk.chunkPosition.x;

                const int tempChunkZ = (z < 0)
                                     ? (chunk.chunkPosition.y - 1)
                                     : (z > Chunk::SIZE - 1)
                                           ? (chunk.chunkPosition.y + 1)
                                           : chunk.chunkPosition.y;

                if (tempChunkX >= 0 && tempChunkX < World::SIZE && tempChunkZ >= 0 && tempChunkZ < World::SIZE) {
                    Chunk* tempChunk = chunk.world.GetChunk(tempChunkX, tempChunkZ);

                    const int tempChunkLocalX = (x < 0) ? (Chunk::SIZE + x) : (x > Chunk::SIZE - 1) ? (x - Chunk::SIZE) : x;

                    const int tempChunkLocalZ = (z < 0) ? (Chunk::SIZE + z) : (z > Chunk::SIZE - 1) ? (z - Chunk::SIZE) : z;

                    if (tempChunk != nullptr && tempChunk->generatedBlockData && !tempChunk->inThread && tempChunk->
                        generatedBuffData) {
                        if (tempChunk->GetBlockID(glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ)) == 0) {
                            tempChunk->SetBlock(glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ), 8);
                            WorldThreading::updateLoadData(tempChunk);
                        }
                    } else if (tempChunk != nullptr && tempChunk->generatedBlockData && !tempChunk->inThread && !
                               tempChunk->generatedBuffData) {
                        if (tempChunk->GetBlockID(glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ)) == 0) {
                            tempChunk->SetBlock(glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ), 8);
                        }
                    } else
                    // if(&tempChunk == nullptr || (&tempChunk != nullptr && (!tempChunk.inThread || (tempChunk.inThread && tempChunk.generatedBlockData))))
                    {
                        WorldThreading::addToBlocksToBeAdded(glm::ivec2(tempChunkX, tempChunkZ), glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ), 8);
                    }
                }
            }
        }
    }
}
void ChunkGeneration::addWaterBorderingChunk() {
    /*
    //for (int x = startX; x < endX; x++) {
        //for (int z = startZ; z < endZ; z++) {
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
                            WorldThreading::updateLoadData(&tempChunk);
                        }
                    } else if (&tempChunk != nullptr && tempChunk.generatedBlockData && !tempChunk.inThread && !
                               tempChunk.generatedBuffData) {
                        if (tempChunk.GetBlockID(glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ)) == 0) {
                            tempChunk.SetBlock(glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ), 8);
                        }
                    } else
                    // if(&tempChunk == nullptr || (&tempChunk != nullptr && (!tempChunk.inThread || (tempChunk.inThread && tempChunk.generatedBlockData))))
                    {
                        WorldThreading::addToBlocksToBeAdded(glm::ivec2(tempChunkX, tempChunkZ), glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ), 8);
                    }
                }
            }
        }
    }*/
}
//WorldThreading::addToBlocksToBeAdded(glm::ivec2(tempChunkX, tempChunkZ), glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ), 8);

void ChunkGeneration::UpdateWater(Chunk &chunk, glm::ivec3 waterPos) {
    const int left = waterPos.x - 1;
    const int right = waterPos.x + 1;
    const int front = waterPos.z - 1;
    const int back = waterPos.z + 1;
    const int down = waterPos.y - 1;

    if (left >= 0) {
        if (chunk.GetBlockID(glm::ivec3(left, waterPos.y, waterPos.z)) == 0) {
            auto block = BlocksToBeAdded(chunk.chunkPosition, glm::ivec3(left, waterPos.y, waterPos.z), 5);
            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    } else {
        auto blockPosInBorderingChunk = glm::ivec3(Chunk::SIZE - 1, waterPos.y, waterPos.z);
        auto borderingChunkPosition = glm::ivec2(chunk.chunkPosition.x - 1, chunk.chunkPosition.y);
        //if(chunk.world.GetChunk(borderingChunkPosition)) {
            //if(!chunk.world.GetChunk(borderingChunkPosition)->generatedBlockData) {
                if (chunk.world.GetChunk(borderingChunkPosition)->GetBlockID(blockPosInBorderingChunk) == 0) {

                    auto block = BlocksToBeAdded(borderingChunkPosition, blockPosInBorderingChunk, 5);

                    if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                        world.liquidToBeChecked.end())
                        chunk.world.liquidToBeChecked.push_back(block);
                }
            //}

        //}


    }
    if (right < Chunk::SIZE) {
        if (chunk.GetBlockID(glm::ivec3(right, waterPos.y, waterPos.z)) == 0) {
            auto block = BlocksToBeAdded(chunk.chunkPosition, glm::ivec3(right, waterPos.y, waterPos.z), 5);
            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    } else {
        auto blockPosInBorderingChunk = glm::ivec3(0, waterPos.y, waterPos.z);
        auto borderingChunkPosition = glm::ivec2(chunk.chunkPosition.x + 1, chunk.chunkPosition.y);

        //if(chunk.world.GetChunk(borderingChunkPosition)) {
            //if (chunk.world.GetChunk(borderingChunkPosition)->GetBlockID(blockPosInBorderingChunk) == 0) {
                if(!chunk.world.GetChunk(borderingChunkPosition)->generatedBlockData) {
                    auto block = BlocksToBeAdded(borderingChunkPosition, blockPosInBorderingChunk, 5);

                    if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                        world.liquidToBeChecked.end())
                        chunk.world.liquidToBeChecked.push_back(block);
                }
            //}
        //}
    }
    if (front >= 0) {
        if (chunk.GetBlockID(glm::ivec3(waterPos.x, waterPos.y, front)) == 0) {
            auto block = BlocksToBeAdded(chunk.chunkPosition, glm::ivec3(waterPos.x, waterPos.y, front), 5);
            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    } else {
        auto blockPosInBorderingChunk = glm::ivec3(waterPos.x, waterPos.y, Chunk::SIZE - 1);
        auto borderingChunkPosition = glm::ivec2(chunk.chunkPosition.x, chunk.chunkPosition.y - 1);

        if (chunk.world.GetChunk(borderingChunkPosition)->GetBlockID(blockPosInBorderingChunk) == 0) {

            auto block = BlocksToBeAdded(borderingChunkPosition, blockPosInBorderingChunk, 5);

            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    }
    if (back < Chunk::SIZE) {
        if (chunk.GetBlockID(glm::ivec3(waterPos.x, waterPos.y, back)) == 0) {
            auto block = BlocksToBeAdded(chunk.chunkPosition, glm::ivec3(waterPos.x, waterPos.y, back), 5);
            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end()) {
                chunk.world.liquidToBeChecked.push_back(block);

                // std::cout<<"\ntest2";
            }
        }
    } else {
        auto blockPosInBorderingChunk = glm::ivec3(waterPos.x, waterPos.y, 0);
        auto borderingChunkPosition = glm::ivec2(chunk.chunkPosition.x, chunk.chunkPosition.y + 1);

        //if(chunk.world.GetChunk(borderingChunkPosition)) {
            //if(!chunk.world.GetChunk(borderingChunkPosition)->generatedBlockData) {
                if (chunk.world.GetChunk(borderingChunkPosition)->GetBlockID(blockPosInBorderingChunk) == 0) {

                    auto block = BlocksToBeAdded(borderingChunkPosition, blockPosInBorderingChunk, 5);

                    if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                        world.liquidToBeChecked.end())
                        chunk.world.liquidToBeChecked.push_back(block);
                }
            //}
        //}
    }
    if (down >= 0) {
        if (chunk.GetBlockID(glm::ivec3(waterPos.x, down, waterPos.z)) == 0) {
            auto block = BlocksToBeAdded(chunk.chunkPosition, glm::ivec3(waterPos.x, down, waterPos.z), 5);
            if (std::find(chunk.world.liquidToBeChecked.begin(), chunk.world.liquidToBeChecked.end(), block) == chunk.
                world.liquidToBeChecked.end())
                chunk.world.liquidToBeChecked.push_back(block);
        }
    }
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

void ChunkGeneration::GenChunk(float *heightMap, const Chunk &chunk) {
    FastNoise noise;

    // Set noise parameters for more varied terrain
    noise.SetNoiseType(FastNoise::PerlinFractal); // Perlin noise for smooth terrain
    noise.SetFractalType(FastNoise::FBM); // FBM for multi-layered noise
    noise.SetFractalOctaves(16); // Increase octaves for more detail
    noise.SetSeed(123); // Example seed
    noise.SetFrequency(0.00835f); // Higher frequency for more hills/valleys



    for (int x = 0; x < Chunk::SIZE; x++) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            constexpr float scaleFactor = 0.65f;

            const float worldX = static_cast<float>(x) + (Chunk::SIZE * static_cast<float>(chunk.chunkPosition.x));
            const float worldZ = static_cast<float>(z) + (Chunk::SIZE * static_cast<float>(chunk.chunkPosition.y));

            const float noiseValue = noise.GetNoise(worldX, worldZ);

            const float terrainHeight = (noiseValue + 1.0f) * 50.0f * scaleFactor;

            heightMap[x + Chunk::SIZE * z] = terrainHeight;
        }
    }
}
