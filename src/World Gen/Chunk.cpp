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
    //std::lock_guard<std::mutex> lock(chunkMutex);
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
    chunkData.chunkNormals.clear();

    chunkData.transparentVerts.clear();
    chunkData.transparentUVs.clear();
    chunkData.transparentIndices.clear();
    chunkData.transparentIndexCount = 0;
    chunkData.transparentBrightnessFloats.clear();
    chunkData.transparentNormals.clear();

    generatedBuffData = false;
}


bool Chunk::compareDistanceToPlayer(const ChunkDataPair& pair1, const ChunkDataPair& pair2, glm::vec3 playerPos) {
    // Calculate the center of each quad and compare distances
    glm::vec3 center1 = (glm::vec3(((pair1.vertices[0]) + (pair1.vertices[1]) + (pair1.vertices[2]) + (pair1.vertices[3]))));
    glm::vec3 center2 = (glm::vec3(((pair2.vertices[0]) + (pair2.vertices[1]) + (pair2.vertices[2]) + (pair2.vertices[3]))));


    //if(glm::distance(playerPos, center1) < glm::distance((playerPos), center2))
        //std::cout<<playerPos.x<<"x "<<playerPos.y<<"y "<<playerPos.z<<"z \n";
        //std::cout<<glm::distance(playerPos, center1)<<"\n"<<glm::distance(playerPos, center2)<<"\n";

    return glm::distance(playerPos, center1) > glm::distance((playerPos), center2);
}
struct Chunk::CompareFaces{
    glm::vec3 playerPos;

    bool operator()(ChunkDataPair pair1, ChunkDataPair pair2){
        //return compareDistanceToPlayer(pair1, pair2, playerPos);

        glm::vec3 center1 = glm::vec3((pair1.vertices[0]) + (pair1.vertices[1]) + (pair1.vertices[2]) + (pair1.vertices[3]))/4.0f;
        glm::vec3 center2 = glm::vec3((pair2.vertices[0]) + (pair2.vertices[1]) + (pair2.vertices[2]) + (pair2.vertices[3]))/4.0f;

        //glm::vec3 normalized1 = glm::normalize(playerPos - center1);
        //glm::vec3 normalized2 = glm::normalize(playerPos - center2);
        double squaredDistance1 = glm::distance((playerPos), (center1));
        double squaredDistance2 = glm::distance((playerPos), (center2));
        return squaredDistance1 > squaredDistance2;

    }
};
void Chunk::sortTransparentMeshData() {
    // Sort transparent mesh data based on distance to player
    CompareFaces compareFaces{};
    glm::vec3 playerPos = *world.camera.position;
    compareFaces.playerPos = playerPos;
    std::vector<ChunkDataPair> combinedData;
    int k = 0;
    for (int i = 0; i < chunkData.transparentVerts.size(); i += 4) {
        ChunkDataPair pair{};
        for (int j = 0; j < 4; j++) {
            pair.vertices[j] = chunkData.transparentVerts[i + j];
            pair.normals[j] = chunkData.transparentNormals[i + j];
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
    //std::cout<<combinedData.size()<<"\n"<<chunkData.transparentVerts.size() / 4<<"\n";
    std::sort(combinedData.begin(), combinedData.end(), compareFaces);

    chunkData.transparentVerts.clear();
    chunkData.transparentUVs.clear();
    chunkData.transparentBrightnessFloats.clear();
    chunkData.transparentIndices.clear();
    chunkData.transparentNormals.clear();

    //k = 0;
    for (int i = 0; i < combinedData.size(); i++) {
        for (int j = 0; j < 4; j++) {

            chunkData.transparentVerts.push_back(combinedData[i].vertices[j]);
            chunkData.transparentNormals.push_back(combinedData[i].normals[j]);
            chunkData.transparentUVs.push_back(combinedData[i].uvs[j]);
            chunkData.transparentBrightnessFloats.push_back(combinedData[i].brightnessFloats[j]);
        }
        for (int j = 0; j < 6; j++) {
            //need to change indices according to verts
            //chunkData.transparentIndices.push_back(combinedData[i].indices[j]);
        }
        //k+=4;
    }
    //chunkData.transparentIndexCount = k;
    chunkData.transparentIndexCount = 0;
    ChunkMeshGeneration::AddIndices(combinedData.size(), chunkData.transparentIndices, chunkData.transparentIndexCount);

    // Now, setData with the sorted data
    //transparentMesh->setData(chunkData.transparentVerts, chunkData.transparentUVs, chunkData.transparentIndices, chunkData.transparentBrightnessFloats);
}
void Chunk::LoadBufferData()
{
    std::lock_guard<std::mutex> lock(chunkMutex);
    if(!chunkHasMeshes){
        mesh = new Mesh();
        transparentMesh = new Mesh();
    }
    if(mesh != nullptr && !inThread && transparentMesh != nullptr)
    {
        mesh->loadedData = false;
        transparentMesh->loadedData = false;
        mesh->setData(chunkData.chunkVerts, chunkData.chunkNormals, chunkData.chunkUVs, chunkData.chunkIndices, chunkData.chunkBrightnessFloats);
        mesh->loadData(*world.scene.shader);
        transparentMesh->setData(chunkData.transparentVerts, chunkData.transparentNormals, chunkData.transparentUVs, chunkData.transparentIndices, chunkData.transparentBrightnessFloats);
        transparentMesh->loadData(*world.scene.transparentShader);
        mesh->loadedData = true;
        transparentMesh->loadedData = true;
    }

}

void Chunk::LoadChunkData() {
    std::lock_guard<std::mutex> lock(chunkMutex);
    ClearVertexData();
    ChunkMeshGeneration::GenFaces(*this);
    ChunkMeshGeneration::UpdateNeighbours(*this);
}

void Chunk::Delete()
{
    std::lock_guard<std::mutex> lock(chunkMutex);

    chunkHasMeshes = false;
    //generatedBuffData = false;
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
