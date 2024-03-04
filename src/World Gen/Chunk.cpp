#include "Chunk.h"
#include "World.h"
#include "ChunkGeneration.h"
#include "ChunkMeshGeneration.h"
#include "Player/Player.h"


Chunk::Chunk(glm::ivec2 Position, World& _world) : world(_world)
{
    chunkPosition = Position;
}

unsigned char Chunk::GetBlockID(glm::ivec3 pos)
{
    if(pos.x < 0 || pos.x > SIZE - 1 || pos.y < 0 || pos.y > HEIGHT - 1 || pos.z < 0 || pos.z > SIZE - 1)
    {
        //std::cout<<"invalid block position at: "<<pos.x<<"x "<<pos.y<<"y "<<pos.z<<"z ";
        return 0;
    }
    else {
        return blockIDs[pos.x + SIZE * (pos.y + HEIGHT * pos.z)];
    }
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

void Chunk::GenBlocks()
{
    ChunkGeneration::GenBlocks(*this);
    generatedBlockData = true;
}

void Chunk::ClearVertexData()
{
    chunkData.indexCount = 0;
    chunkData.chunkVerts.clear();
    chunkData.chunkUVs.clear();
    chunkData.chunkIndices.clear();
    chunkData.chunkBrightnessFloats.clear();

    chunkData.transparentVerts.clear();
    chunkData.transparentUVs.clear();
    chunkData.transparentIndices.clear();
    chunkData.transparentIndexCount = 0;
    chunkData.transparentBrightnessFloats.clear();

    generatedBuffData = false;
}


bool Chunk::compareDistanceToPlayer(const ChunkDataPair& pair1, const ChunkDataPair& pair2) {
    // Calculate the center of each quad and compare distances
    glm::vec3 center1 = glm::vec3((pair1.vertices[0] + pair1.vertices[1] + pair1.vertices[2] + pair1.vertices[3])/4.0f);
    glm::vec3 center2 = glm::vec3((pair2.vertices[0] + pair2.vertices[1] + pair2.vertices[2] + pair2.vertices[3])/4.0f);

    return this->world.player.distanceToPlayer(center1) > this->world.player.distanceToPlayer(center2);
}

void Chunk::sortTransparentMeshData(ChunkData& chunkData, const Player& player) {
    // Sort transparent mesh data based on distance to player
    std::vector<ChunkDataPair> combinedData;
    int k = 0;
    for (size_t i = 0; i < chunkData.transparentVerts.size(); i += 4) {
        ChunkDataPair pair;
        for (int j = 0; j < 4; j++) {
            pair.vertices[j] = chunkData.transparentVerts[i + j];
            pair.brightnessFloats[j] = chunkData.transparentBrightnessFloats[i+j];
            pair.uvs[j] = chunkData.transparentUVs[i + j];
        }
        for(int j = 0; j < 6; j++)
        {
            pair.indices[j] = chunkData.transparentIndices[k + j];
        }
        combinedData.push_back(pair);
        k+=6;
    }
    std::sort(combinedData.begin(), combinedData.end(),
              [this](const ChunkDataPair& vertex1, const ChunkDataPair& vertex2) {
                  return compareDistanceToPlayer(vertex1, vertex2);
              });
    for (size_t i = 0; i < combinedData.size(); i++) {
        for (int j = 0; j < 4; j++) {

            chunkData.transparentVerts[(i * 4) + j] = combinedData[i].vertices[j];
            chunkData.transparentUVs[(i * 4) + j] = combinedData[i].uvs[j];
            chunkData.transparentBrightnessFloats[i * 4 + j] = combinedData[i].brightnessFloats[j];
        }
        for (int j = 0; j < 6; j++) {
            chunkData.transparentIndices[(i * 6) + j] = combinedData[i].indices[j];
        }
    }


    // Now, setData with the sorted data
    transparentMesh->setData(chunkData.transparentVerts, chunkData.transparentUVs, chunkData.transparentIndices, chunkData.transparentBrightnessFloats);
}
void Chunk::LoadBufferData()
{
    if(mesh != nullptr)
    {
        delete mesh;
    }
    mesh = new Mesh(*world.shader);
    mesh->setData(chunkData.chunkVerts, chunkData.chunkUVs, chunkData.chunkIndices, chunkData.chunkBrightnessFloats);
    mesh->loadData();

    if(transparentMesh != nullptr)
    {
        delete transparentMesh;
    }
    transparentMesh = new Mesh(*world.transparentShader);
    //sort from back to front from player pos
    sortTransparentMeshData(chunkData, world.player);
    //transparentMesh->setData(chunkData.transparentVerts, chunkData.transparentUVs, chunkData.transparentIndices, chunkData.transparentBrightnessFloats);
    transparentMesh->loadData();

}

void Chunk::RenderChunk()
{
    if(mesh != nullptr && transparentMesh != nullptr) {
        mesh->render();
        //glDepthMask(GL_FALSE);
        //glDisable(GL_CULL_FACE);
        //glCullFace(GL_BACK);
        transparentMesh->render();
        //glCullFace(GL_FRONT);
        //glEnable(GL_CULL_FACE);
        //glDepthMask(GL_TRUE);
    }
}

void Chunk::LoadChunkData() {
    ClearVertexData();
    ChunkMeshGeneration::GenFaces(*this);
    ChunkMeshGeneration::UpdateNeighbours(*this);
}

void Chunk::Delete()
{
    chunkHasMeshes = false;
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
