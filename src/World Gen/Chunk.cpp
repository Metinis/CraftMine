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
bool Chunk::compareDistanceToPlayer(const glm::vec3& vertex1, const glm::vec3& vertex2) {
    return world.player.distanceToPlayer(vertex1) > world.player.distanceToPlayer(vertex2);
}
struct ChunkDataPair{
    glm::vec3 vertex;
    glm::vec2 uv;
    std::vector<GLuint> indices;
    std::vector<float> brightnessFloats;
};
void Chunk::sortTransparentMeshData(ChunkData& chunkData, const Player& player) {
    // Sort transparent mesh data based on distance to player
    std::vector<ChunkDataPair> combinedData;
    int k = 0;
    for (size_t i = 0; i < chunkData.transparentVerts.size(); ++i) {
        ChunkDataPair pair;
        pair.vertex = chunkData.transparentVerts[i];
        pair.uv = chunkData.transparentUVs[i];
        //for(int j = k; j < k + 4; j++)
        //{
        //    pair.indices.push_back(chunkData.transparentIndices[j]);
        //    pair.brightnessFloats.push_back(chunkData.transparentBrightnessFloats[j]);
        //}
        combinedData.push_back(pair);
        //k+=4;
    }
    std::sort(combinedData.begin(), combinedData.end(),
              [this](const ChunkDataPair& vertex1, const ChunkDataPair& vertex2) {
                  return compareDistanceToPlayer(vertex1.vertex, vertex2.vertex);
              });
    k = 0;
    for (size_t i = 0; i < combinedData.size(); ++i) {
        //chunkData.transparentVerts[i] = combinedData[i].vertex;
        //chunkData.transparentUVs[i] = combinedData[i].uv;
        //for(int j = k; j < k + 4; j++)
        //{
        //    chunkData.transparentIndices = combinedData[i].indices;
        //    chunkData.transparentBrightnessFloats = combinedData[i].brightnessFloats;
        //}
        //k+=4;
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
    //sortTransparentMeshData(chunkData, world.player);
    transparentMesh->setData(chunkData.transparentVerts, chunkData.transparentUVs, chunkData.transparentIndices, chunkData.transparentBrightnessFloats);
    transparentMesh->loadData();

}

void Chunk::RenderChunk()
{
    if(mesh != nullptr && transparentMesh != nullptr) {
        mesh->render();
        //glDepthMask(GL_FALSE);
        transparentMesh->render();
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
