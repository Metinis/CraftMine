#include "Chunk.h"
#include "World.h"
#include "ChunkGeneration.h"
#include "ChunkMeshGeneration.h"


Chunk::Chunk(const glm::ivec2 Position, World& _world) : world(_world)
{
    chunkPosition = Position;
}

unsigned char Chunk::GetBlockID(const glm::ivec3 pos) const
{
    if (pos.x < 0 || pos.x > SIZE - 1 || pos.y < 0 || pos.y > HEIGHT - 1 || pos.z < 0 || pos.z > SIZE - 1) {
        return 0;
    }

    return blockIDs[pos.x + SIZE * (pos.y + HEIGHT * pos.z)];
}


void Chunk::SetBlock(const glm::ivec3 pos, const unsigned char id)
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
    chunkData.indexCount = 0;
    chunkData.chunkVerts.clear();
    chunkData.chunkUVs.clear();
    chunkData.chunkIndices.clear();
    chunkData.chunkBrightnessFloats.clear();
    chunkData.chunkNormals.clear();

    chunkData.nonSolidVerts.clear();
    chunkData.nonSolidUVs.clear();
    chunkData.nonSolidIndices.clear();
    chunkData.nonSolidIndexCount = 0;
    chunkData.nonSolidBrightnessFloats.clear();
    chunkData.nonSolidNormals.clear();

    generatedBuffData = false;
}


bool Chunk::compareDistanceToPlayer(const ChunkDataPair& pair1, const ChunkDataPair& pair2, glm::vec3 playerPos) {
    // Calculate the center of each quad and compare distances
    const auto center1 = (glm::vec3(((pair1.vertices[0]) + (pair1.vertices[1]) + (pair1.vertices[2]) + (pair1.vertices[3]))));
    const auto center2 = (glm::vec3(((pair2.vertices[0]) + (pair2.vertices[1]) + (pair2.vertices[2]) + (pair2.vertices[3]))));

    return glm::distance(playerPos, center1) > glm::distance((playerPos), center2);
}
struct Chunk::CompareFaces{

    glm::vec3 playerPos;

    bool operator()(const ChunkDataPair &pair1, const ChunkDataPair &pair2) const{

        const auto center1 = glm::vec3((pair1.vertices[0]) + (pair1.vertices[1]) + (pair1.vertices[2]) + (pair1.vertices[3]))/4.0f;
        const auto center2 = glm::vec3((pair2.vertices[0]) + (pair2.vertices[1]) + (pair2.vertices[2]) + (pair2.vertices[3]))/4.0f;

        const double squaredDistance1 = glm::distance((playerPos), (center1));
        const double squaredDistance2 = glm::distance((playerPos), (center2));
        return squaredDistance1 > squaredDistance2;

    }
};

void Chunk::sortTransparentMeshData() {
    std::lock_guard<std::mutex> lock(chunkMeshMutex);
    if(!inThread && !toBeDeleted) {
        // Sort transparent mesh data based on distance to player
        CompareFaces compareFaces{};
        const glm::vec3 cameraPos = world.camera.position;
        compareFaces.playerPos = cameraPos;
        std::vector<ChunkDataPair> combinedData;
        int k = 0;
        for (int i = 0; i < chunkData.nonSolidVerts.size(); i += 4) {
            ChunkDataPair pair{};
            for (int j = 0; j < 4; j++) {
                pair.vertices[j] = chunkData.nonSolidVerts[i + j];
                pair.normals[j] = chunkData.nonSolidNormals[i + j];
                pair.brightnessFloats[j] = chunkData.nonSolidBrightnessFloats[i + j];
                pair.uvs[j] = chunkData.nonSolidUVs[i + j];
            }
            for (int j = 0; j < 6; j++) {
                pair.indices[j] = chunkData.nonSolidIndices[k + j];
            }
            combinedData.push_back(pair);
            k += 6;
        }
        std::sort(combinedData.begin(), combinedData.end(), compareFaces);

        chunkData.nonSolidVerts.clear();
        chunkData.nonSolidUVs.clear();
        chunkData.nonSolidBrightnessFloats.clear();
        chunkData.nonSolidIndices.clear();
        chunkData.nonSolidNormals.clear();

        for (auto & i : combinedData) {
            for (int j = 0; j < 4; j++) {

                chunkData.nonSolidVerts.push_back(i.vertices[j]);
                chunkData.nonSolidNormals.push_back(i.normals[j]);
                chunkData.nonSolidUVs.push_back(i.uvs[j]);
                chunkData.nonSolidBrightnessFloats.push_back(i.brightnessFloats[j]);
            }
        }
        chunkData.nonSolidIndexCount = 0;
        ChunkMeshGeneration::AddIndices(static_cast<int>(combinedData.size()), chunkData.nonSolidIndices,
                                        chunkData.nonSolidIndexCount);
    }
}
void Chunk::sortTransparentMeshData(glm::vec3 position) {
    std::lock_guard<std::mutex> lock(chunkMeshMutex);
    // Sort transparent mesh data based on distance to player
    CompareFaces compareFaces{};
    compareFaces.playerPos = position;
    std::vector<ChunkDataPair> combinedData;
    int k = 0;
    for (int i = 0; i < chunkData.nonSolidVerts.size(); i += 4) {
        ChunkDataPair pair{};
        for (int j = 0; j < 4; j++) {
            pair.vertices[j] = chunkData.nonSolidVerts[i + j];
            pair.normals[j] = chunkData.nonSolidNormals[i + j];
            pair.brightnessFloats[j] = chunkData.nonSolidBrightnessFloats[i+j];
            pair.uvs[j] = chunkData.nonSolidUVs[i + j];
        }
        for(int j = 0; j < 6; j++)
        {
            pair.indices[j] = chunkData.nonSolidIndices[k + j];
        }
        combinedData.push_back(pair);
        k+=6;
    }
    std::sort(combinedData.begin(), combinedData.end(), compareFaces);

    chunkData.nonSolidVerts.clear();
    chunkData.nonSolidUVs.clear();
    chunkData.nonSolidBrightnessFloats.clear();
    chunkData.nonSolidIndices.clear();
    chunkData.nonSolidNormals.clear();

    for (auto & i : combinedData) {
        for (int j = 0; j < 4; j++) {

            chunkData.nonSolidVerts.push_back(i.vertices[j]);
            chunkData.nonSolidNormals.push_back(i.normals[j]);
            chunkData.nonSolidUVs.push_back(i.uvs[j]);
            chunkData.nonSolidBrightnessFloats.push_back(i.brightnessFloats[j]);
        }
    }
    chunkData.nonSolidIndexCount = 0;
    ChunkMeshGeneration::AddIndices(static_cast<int>(combinedData.size()), chunkData.nonSolidIndices, chunkData.nonSolidIndexCount);
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
    }

    if(mesh != nullptr && !inThread && transparentMesh != nullptr)
    {
        mesh->setData(chunkData.chunkVerts, chunkData.chunkNormals, chunkData.chunkUVs, chunkData.chunkIndices, chunkData.chunkBrightnessFloats);
        mesh->loadData(*world.scene.geometryShader);
        transparentMesh->setData(chunkData.nonSolidVerts, chunkData.nonSolidNormals, chunkData.nonSolidUVs, chunkData.nonSolidIndices, chunkData.nonSolidBrightnessFloats);
        transparentMesh->loadData(*world.scene.geometryShader);
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
    std::lock_guard<std::mutex> lock(chunkMeshMutex);
    return chunkBools.rightSideUpdated && chunkBools.leftSideUpdated && chunkBools.frontUpdated && chunkBools.backUpdated;
}



std::string Chunk::getRegionFilename(const int regionX, const int regionY) {
    return SOURCE_DIR "/save/chunkData/" + std::to_string(regionX) + "-" + std::to_string(regionY) + ".bin";
}

int Chunk::getChunkOffset(const int chunkX, const int chunkY) {
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

        const int regionX = chunkPosition.x / CHUNKS_PER_REGION;
        const int regionY = chunkPosition.y / CHUNKS_PER_REGION;
        const std::string filename = getRegionFilename(regionX, regionY);

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

        const int chunkOffset = getChunkOffset(chunkPosition.x, chunkPosition.y);
        const int dataOffset = static_cast<int>(chunkOffset * sizeof(blockIDs));
        outfile.seekp(dataOffset);
        outfile.write(reinterpret_cast<const char*>(compressedData.data()), static_cast<long>(compressedSize));
        outfile.close();
    }
}

bool Chunk::loadData() {
    std::lock_guard<std::mutex> lock(chunkBlockMutex);

    const int regionX = chunkPosition.x / CHUNKS_PER_REGION;
    const int regionY = chunkPosition.y / CHUNKS_PER_REGION;
    const std::string filename = getRegionFilename(regionX, regionY);

    std::ifstream infile(filename, std::ios::binary);
    if (!infile) {
        return false;
    }

    const int chunkOffset = getChunkOffset(chunkPosition.x, chunkPosition.y);
    const int dataOffset = static_cast<int>(chunkOffset * sizeof(blockIDs));
    infile.seekg(dataOffset);

    std::vector<unsigned char> compressedData(sizeof(blockIDs));
    infile.read(reinterpret_cast<char*>(compressedData.data()), sizeof(blockIDs));
    infile.close();

    uLongf decompressedSize = sizeof(blockIDs);
    const int result = uncompress(blockIDs, &decompressedSize, compressedData.data(), sizeof(blockIDs));

    if (result != Z_OK) {
        return false;
    }

    if (decompressedSize != sizeof(blockIDs)) {
        return false;
    }

    return true;
}
// Calculate min and max bounds of the chunk in world space
glm::vec3 Chunk::getChunkMinBounds() const {
    return {
        chunkPosition.x * Chunk::SIZE,
        0,
        chunkPosition.y * Chunk::SIZE
    };

}

glm::vec3 Chunk::getChunkMaxBounds() const {
    return {
        chunkPosition.x * Chunk::SIZE + Chunk::SIZE,
        Chunk::HEIGHT,
        chunkPosition.y * Chunk::SIZE + Chunk::SIZE
    };
}
