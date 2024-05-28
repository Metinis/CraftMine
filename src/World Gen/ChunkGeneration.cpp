//
// Created by vodkasas on 03/02/24.
//

#include "ChunkGeneration.h"
void ChunkGeneration::GenBlocks(Chunk& chunk){
    float heightMap[Chunk::HEIGHT * Chunk::HEIGHT];

    GenChunk(heightMap, chunk);

    for (int i = 0; i < Chunk::SIZE * Chunk::SIZE * Chunk::HEIGHT; i++)
    {
        int x = i % Chunk::SIZE;
        int y = i / (Chunk::SIZE * Chunk::SIZE);
        int z = (i / Chunk::SIZE) % Chunk::SIZE;

        int seaLevel = Chunk::HEIGHT / 2;

        int columnHeight = (int)heightMap[x + Chunk::SIZE * z] + (seaLevel -20); //add sealevel so noise generates above it

        unsigned char id = 0;
        if(chunk.GetBlockID(glm::ivec3(x,y,z)) == 0) {    //so that trees/structures don't get overwritten
            if (y >= columnHeight && y + 2 < seaLevel) { //last argument is the sea level

                id = 5;
            } else if (y <= columnHeight - 1 && y >= columnHeight - 3 && y < seaLevel - 2) {
                id = 6; //sand
            } else if (y == columnHeight - 1) {
                id = 1;                        //Refer to id map, grass layer
                //check to generate tree
                if (shouldGenTree()) {
                    id = 2; //if there's tree above, make dirt block, not grass
                    genTree(glm::vec3(x, y+1, z), chunk);
                }
            } else if (y > columnHeight - 4 && y < columnHeight) {
                id = 2;                        //dirt layer
            } else if (y <= columnHeight - 4 && y > 0) {
                id = 3;                        //stone layer
            } else if (y == 0) {
                id = 4;                        //bedrock layer
            }

            chunk.SetBlock(glm::ivec3(x,y,z), id);
        }
    }
}
bool ChunkGeneration::shouldGenTree()
{

    std::mt19937 rng(std::random_device{}());
    // Define the probability (1 in 250 chance)
    std::uniform_int_distribution<> dis(1, 250);

    // Generate a random number
    int randomNumber = dis(rng);

    // Check if the random number falls within the desired range
    return randomNumber == 1;
}
void ChunkGeneration::genTree(glm::ivec3 treeCoord, Chunk& chunk) //starts from bottom block
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

    for(int y = treeCoord.y; y < (treeHeight + treeCoord.y + leafHeight) && y < Chunk::HEIGHT; y++)
    {
        int localY = y - treeCoord.y;
        //set the tree
        if(y < treeHeight + treeCoord.y)
            chunk.SetBlock(glm::ivec3(treeCoord.x, y, treeCoord.z), 7); //7 is wood

        if(localY >= treeHeight - 2 && localY < treeHeight) {
            //bottom layer
            int startX = treeCoord.x - 2;
            int endX = treeCoord.x + 3;
            int startZ = treeCoord.z - 2;
            int endZ = treeCoord.z + 3;

            generateLeaves(startX, endX, startZ, endZ, y, chunk);
        }
            //top layer
        else if(localY >= treeHeight && localY < (treeHeight + leafHeight))
        {
            int startX = treeCoord.x - 1;
            int endX = treeCoord.x + 2;
            int startZ = treeCoord.z - 1;
            int endZ = treeCoord.z + 2;

            generateLeaves(startX, endX, startZ, endZ, y, chunk);
        }



    }
}
void ChunkGeneration::generateLeaves(int startX, int endX, int startZ, int endZ, int y, Chunk& chunk)
{
    for (int x = startX; x < endX; x++) {
        for (int z = startZ; z < endZ; z++) {
            //if x and z are in this chunk
            if(x >= 0 && x < Chunk::SIZE && z >= 0 && z < Chunk::SIZE)
            {
                if (chunk.GetBlockID(glm::ivec3(x,y,z)) == 0)
                    chunk.SetBlock(glm::ivec3(x,y,z), 8);
            }
            else
            {
                //find the chunk x and z are in, check if it is not null, convert x and y to that local chunk coordinates and set blocks and update buffers
                //Add to loaded chunks for updating

                int tempChunkX = (x < 0) ? (chunk.chunkPosition.x - 1) :
                                 (x > Chunk::SIZE - 1) ? (chunk.chunkPosition.x + 1) :
                                 chunk.chunkPosition.x;

                int tempChunkZ = (z < 0) ? (chunk.chunkPosition.y - 1) :
                                 (z > Chunk::SIZE - 1) ? (chunk.chunkPosition.y + 1) :
                                 chunk.chunkPosition.y;

                if(tempChunkX >= 0 && tempChunkX < World::SIZE && tempChunkZ >= 0 && tempChunkZ < World::SIZE)
                {
                    Chunk& tempChunk = *chunk.world.GetChunk(tempChunkX, tempChunkZ);

                    int tempChunkLocalX = (x < 0) ? (Chunk::SIZE + x) :
                                          (x > 15) ? (x - Chunk::SIZE) :
                                          x;

                    int tempChunkLocalZ = (z < 0) ? (Chunk::SIZE + z) :
                                          (z > 15) ? (z - Chunk::SIZE) :
                                          z;

                    if(&tempChunk != nullptr && tempChunk.generatedBlockData && !tempChunk.inThread && tempChunk.generatedBuffData)
                    {
                        if(tempChunk.GetBlockID(glm::ivec3(tempChunkLocalX,y,tempChunkLocalZ)) == 0) {
                            tempChunk.SetBlock(glm::ivec3(tempChunkLocalX,y,tempChunkLocalZ), 8);
                            chunk.world.chunksToLoadData.push_back(&tempChunk);
                        }
                    }
                    else if(&tempChunk != nullptr && tempChunk.generatedBlockData && !tempChunk.inThread && !tempChunk.generatedBuffData)
                    {
                        if(tempChunk.GetBlockID(glm::ivec3(tempChunkLocalX,y,tempChunkLocalZ)) == 0) {
                            tempChunk.SetBlock(glm::ivec3(tempChunkLocalX,y,tempChunkLocalZ), 8);
                        }
                    }
                    else// if(&tempChunk == nullptr || (&tempChunk != nullptr && (!tempChunk.inThread || (tempChunk.inThread && tempChunk.generatedBlockData))))
                    {
                        chunk.world.mutexBlocksToBeAddedList.lock();
                        chunk.world.blocksToBeAddedList.push_back(BlocksToBeAdded{glm::ivec2(tempChunkX,tempChunkZ), glm::ivec3(tempChunkLocalX, y, tempChunkLocalZ), 8});
                        chunk.world.mutexBlocksToBeAddedList.unlock();
                    }

                }

            }

        }
    }
}
void ChunkGeneration::GenChunk(float* heightMap, Chunk& chunk)	//might need to be amended to include more chunks
{
    FastNoise noise;

    // Set noise parameters
    noise.SetNoiseType(FastNoise::SimplexFractal);
    noise.SetFractalType(FastNoise::FBM);
    noise.SetFractalOctaves(4);
    noise.SetSeed(100); //123 is old seed
    noise.SetFractalLacunarity(2.0f);
    noise.SetFrequency(0.0075f);
    float scaleFactor = 0.25f;

    for (int x = 0; x < Chunk::SIZE; x++)
    {
        for (int z = 0; z < Chunk::SIZE; z++)
        {

            float noiseValue = noise.GetNoise((float)x + (Chunk::SIZE * (float)chunk.chunkPosition.x), (float)z + (Chunk::SIZE * (float)chunk.chunkPosition.y));
            int terrainHeight = static_cast<int>((noiseValue + 1.0) * 100.0f * scaleFactor); //100.0f represents the highest point
            heightMap[x + Chunk::SIZE * z] = terrainHeight;
        }
    }
}