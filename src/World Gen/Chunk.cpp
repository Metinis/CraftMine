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

        //std::lock_guard<std::mutex> lock(chunkBlockMutex);

        //std::lock_guard<std::mutex> _lock(chunkDeleteMutex);
    if (pos.x < 0 || pos.x > SIZE - 1 || pos.y < 0 || pos.y > HEIGHT - 1 || pos.z < 0 || pos.z > SIZE - 1) {
        //std::cout<<"invalid block position at: "<<pos.x<<"x "<<pos.y<<"y "<<pos.z<<"z ";
        return 0;
    } else {
        return blockIDs[pos.x + SIZE * (pos.y + HEIGHT * pos.z)];
    }
        //chunkDeleteMutex.unlock();
}

void Chunk::SetBlock(glm::ivec3 pos, unsigned char id)
{
    {

        std::lock_guard<std::mutex> lock(chunkBlockMutex);
        if (pos.x < 0 || pos.x > SIZE - 1 || pos.y < 0 || pos.y > HEIGHT - 1 || pos.z < 0 || pos.z > SIZE - 1) {
            std::cout << "invalid block position at: " << pos.x << "x " << pos.y << "y " << pos.z << "z ";
        } else {
            blockIDs[pos.x + SIZE * (pos.y + HEIGHT * pos.z)] = id;
        }
    }

    saveData();
}

void Chunk::GenBlocks()
{
    {
        if (!loadData()) {
            ChunkGeneration::GenBlocks(*this);
        }
        generatedBlockData = true;
    }
    saveData();
}

void Chunk::ClearVertexData()
{
    //std::lock_guard<std::mutex> lock(chunkMeshMutex);
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

    return glm::distance(playerPos, center1) > glm::distance((playerPos), center2);
}
struct Chunk::CompareFaces{

    glm::vec3 playerPos;

    bool operator()(ChunkDataPair pair1, ChunkDataPair pair2){

        glm::vec3 center1 = glm::vec3((pair1.vertices[0]) + (pair1.vertices[1]) + (pair1.vertices[2]) + (pair1.vertices[3]))/4.0f;
        glm::vec3 center2 = glm::vec3((pair2.vertices[0]) + (pair2.vertices[1]) + (pair2.vertices[2]) + (pair2.vertices[3]))/4.0f;

        double squaredDistance1 = glm::distance((playerPos), (center1));
        double squaredDistance2 = glm::distance((playerPos), (center2));
        return squaredDistance1 > squaredDistance2;

    }
};

void Chunk::sortTransparentMeshData() {
    std::lock_guard<std::mutex> lock(chunkMeshMutex);
    if(!inThread && !toBeDeleted) {
        // Sort transparent mesh data based on distance to player
        CompareFaces compareFaces{};
        glm::vec3 cameraPos = world.camera.position;
        compareFaces.playerPos = cameraPos;
        std::vector<ChunkDataPair> combinedData;
        int k = 0;
        for (int i = 0; i < chunkData.transparentVerts.size(); i += 4) {
            ChunkDataPair pair{};
            for (int j = 0; j < 4; j++) {
                pair.vertices[j] = chunkData.transparentVerts[i + j];
                pair.normals[j] = chunkData.transparentNormals[i + j];
                pair.brightnessFloats[j] = chunkData.transparentBrightnessFloats[i + j];
                pair.uvs[j] = chunkData.transparentUVs[i + j];
            }
            for (int j = 0; j < 6; j++) {
                pair.indices[j] = chunkData.transparentIndices[k + j];
            }
            combinedData.push_back(pair);
            k += 6;
        }
        std::sort(combinedData.begin(), combinedData.end(), compareFaces);

        chunkData.transparentVerts.clear();
        chunkData.transparentUVs.clear();
        chunkData.transparentBrightnessFloats.clear();
        chunkData.transparentIndices.clear();
        chunkData.transparentNormals.clear();

        for (int i = 0; i < combinedData.size(); i++) {
            for (int j = 0; j < 4; j++) {

                chunkData.transparentVerts.push_back(combinedData[i].vertices[j]);
                chunkData.transparentNormals.push_back(combinedData[i].normals[j]);
                chunkData.transparentUVs.push_back(combinedData[i].uvs[j]);
                chunkData.transparentBrightnessFloats.push_back(combinedData[i].brightnessFloats[j]);
            }
        }
        chunkData.transparentIndexCount = 0;
        ChunkMeshGeneration::AddIndices(combinedData.size(), chunkData.transparentIndices,
                                        chunkData.transparentIndexCount);
    }
}
void Chunk::sortTransparentMeshData(glm::vec3 position) {
    std::lock_guard<std::mutex> lock(chunkMeshMutex);
    // Sort transparent mesh data based on distance to player
    CompareFaces compareFaces{};
    compareFaces.playerPos = position;
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
    }
    chunkData.transparentIndexCount = 0;
    ChunkMeshGeneration::AddIndices(combinedData.size(), chunkData.transparentIndices, chunkData.transparentIndexCount);
}
void Chunk::LoadBufferData()
{

    std::lock_guard<std::mutex> lock(chunkMeshMutex);
    generatedBuffData = false;

    if(!chunkHasMeshes){
        if(mesh == nullptr){
            mesh = new Mesh();
        }
        if(transparentMesh == nullptr){
            transparentMesh = new Mesh();
        }
        //chunkHasMeshes = true;
    }

    if(mesh != nullptr && !inThread && transparentMesh != nullptr)
    {
        //mesh->loadedData = false;
        //transparentMesh->loadedData = false;

        // Add further checks to ensure sizes are within expected ranges
        if (chunkData.chunkVerts.size() >= 0 && chunkData.chunkNormals.size() >= 0 &&
            chunkData.chunkUVs.size() >= 0 && chunkData.chunkIndices.size() >= 0 &&
            chunkData.chunkBrightnessFloats.size() >= 0 &&
            chunkData.transparentVerts.size() >= 0 && chunkData.transparentNormals.size() >= 0 &&
            chunkData.transparentUVs.size() >= 0 && chunkData.transparentIndices.size() >= 0 &&
            chunkData.transparentBrightnessFloats.size() >= 0) {

            mesh->setData(chunkData.chunkVerts, chunkData.chunkNormals, chunkData.chunkUVs, chunkData.chunkIndices, chunkData.chunkBrightnessFloats);
            mesh->loadData(*world.scene.geometryShader);
            transparentMesh->setData(chunkData.transparentVerts, chunkData.transparentNormals, chunkData.transparentUVs, chunkData.transparentIndices, chunkData.transparentBrightnessFloats);
            transparentMesh->loadData(*world.scene.geometryShader);

            //mesh->loadedData = true;
            //transparentMesh->loadedData = true;
        } else {
            return;
        }
    }
    generatedBuffData = true;
}

void Chunk::LoadChunkData() {
    std::lock_guard<std::mutex> lock(chunkMeshMutex);
    ClearVertexData();
    ChunkMeshGeneration::GenFaces(*this);
    ChunkMeshGeneration::UpdateNeighbours(*this);
}

void Chunk::Delete()
{
    if(mesh != nullptr){
        delete mesh;
        mesh = nullptr;
    }
    if(transparentMesh != nullptr) {

        delete transparentMesh;
        transparentMesh = nullptr;
    }
}

Chunk::~Chunk()
{
    {
        std::lock_guard<std::mutex> lock(chunkMeshMutex);
        std::lock_guard<std::mutex> _lock(chunkDeleteMutex);
        std::lock_guard<std::mutex> alock(chunkBlockMutex);
        Delete();
    }
}

bool Chunk::getIsAllSidesUpdated() {
    //
     std::lock_guard<std::mutex> lock(chunkMeshMutex);
    return chunkBools.rightSideUpdated && chunkBools.leftSideUpdated && chunkBools.frontUpdated && chunkBools.backUpdated;
}



std::string Chunk::getRegionFilename(int regionX, int regionY) {
    return "../save/chunkData/" + std::to_string(regionX) + "-" + std::to_string(regionY) + ".bin";
}

int Chunk::getChunkOffset(int chunkX, int chunkY) {
    return (chunkY % CHUNKS_PER_REGION) * CHUNKS_PER_REGION + (chunkX % CHUNKS_PER_REGION);
}

void Chunk::saveData() {
    std::lock_guard<std::mutex> lock(chunkBlockMutex);
    if (generatedBlockData) {
        uLongf compressedSize = compressBound(sizeof(blockIDs));
        std::vector<unsigned char> compressedData(compressedSize);

        int result = compress(compressedData.data(), &compressedSize, blockIDs, sizeof(blockIDs));
        if (result != Z_OK) {
            std::cerr << "Failed to compress data" << std::endl;
            return;
        }

        int regionX = chunkPosition.x / CHUNKS_PER_REGION;
        int regionY = chunkPosition.y / CHUNKS_PER_REGION;
        std::string filename = getRegionFilename(regionX, regionY);

        std::ofstream outfile(filename, std::ios::binary | std::ios::in | std::ios::out);
        if (!outfile) {
            // If the file doesn't exist, create it
            outfile.open(filename, std::ios::binary | std::ios::out);
            if (!outfile) {
                std::cerr << "Failed to create region file: " << filename << std::endl;
                return;
            }
            outfile.close();
            outfile.open(filename, std::ios::binary | std::ios::in | std::ios::out);
        }

        int chunkOffset = getChunkOffset(chunkPosition.x, chunkPosition.y);
        int dataOffset = chunkOffset * sizeof(blockIDs);
        outfile.seekp(dataOffset);
        outfile.write(reinterpret_cast<const char*>(compressedData.data()), compressedSize);
        outfile.close();
    }
}

bool Chunk::loadData() {
    std::lock_guard<std::mutex> lock(chunkBlockMutex);

    int regionX = chunkPosition.x / CHUNKS_PER_REGION;
    int regionY = chunkPosition.y / CHUNKS_PER_REGION;
    std::string filename = getRegionFilename(regionX, regionY);

    std::ifstream infile(filename, std::ios::binary);
    if (!infile) {
        return false;
    }

    int chunkOffset = getChunkOffset(chunkPosition.x, chunkPosition.y);
    int dataOffset = chunkOffset * sizeof(blockIDs);
    infile.seekg(dataOffset);

    std::vector<unsigned char> compressedData(sizeof(blockIDs));
    infile.read(reinterpret_cast<char*>(compressedData.data()), sizeof(blockIDs));
    infile.close();

    uLongf decompressedSize = sizeof(blockIDs);
    int result = uncompress(blockIDs, &decompressedSize, compressedData.data(), sizeof(blockIDs));

    if (result != Z_OK) {
       // std::cerr << "Failed to decompress data" << std::endl;
        return false;
    }

    if (decompressedSize != sizeof(blockIDs)) {
      //  std::cerr << "Decompressed data size does not match expected size" << std::endl;
        return false;
    }

    return true;
}
