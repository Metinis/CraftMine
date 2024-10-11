#include "Block.h"
#include "TextureData.h"

FaceData Block::GetFace(Faces face, BlockType type, glm::vec3 position)
{
    if(type == CraftMine::EMPTY)
    {
        std::cout<<"empty block \n";
    }
    //TODO update this check for custom mesh
    if(type == ROSE_FLOWER || type == DANDELION_FLOWER){
        return FaceData{ AddTransformedVertices(FaceDataRaw::rawVertexDataFlower.at(face), position, type), TextureData::GetBlockTypeUV(type, face),
                         CraftMine::brightnessMap.at(face), CraftMine::normalsMap.at(face) };
    }
	return FaceData{ AddTransformedVertices(FaceDataRaw::rawVertexData.at(face), position, type), TextureData::GetBlockTypeUV(type, face),
                     CraftMine::brightnessMap.at(face), CraftMine::normalsMap.at(face) };
}
std::vector<glm::vec3> Block::GetOutline(glm::vec3 position, unsigned char id)
{
    std::vector<glm::vec3> newVertices;
    auto it = customMeshDataMap.find(id);
    if (it != customMeshDataMap.end()) {
        const auto &faceDataMap = it->second;
        for (const auto &faceData: faceDataMap) {
            std::vector<glm::vec3> vertices = AddOutlineVertices(faceData.second, position, true);
            newVertices.insert(newVertices.end(), vertices.begin(), vertices.end());
        }
    }
    else{
        for (int i = 0; i < 6; i++) {
            Faces currentFace = static_cast<Faces>(i);
            std::vector<glm::vec3> vertices = AddOutlineVertices(FaceDataRaw::rawVertexData.at(currentFace),
                                                                 position, false);
            newVertices.insert(newVertices.end(), vertices.begin(), vertices.end());
        }
    }

    return newVertices;
}
std::vector<glm::vec3>
Block::AddOutlineVertices(std::vector<glm::vec3> vertices, glm::vec3 Position, bool hasCustomMesh)
{
    std::vector<glm::vec3> newVerts;

    for (glm::vec3 vert : vertices)
    {
        vert = vert * 1.01f;
        if(hasCustomMesh){
            vert.x /= 3;
            vert.z /= 3;
            if (vert.y > 0){
                vert.y /= 3;
            }
        }
        newVerts.push_back(vert + Position);
    }
    return newVerts;
}
std::vector<glm::vec3> Block::AddTransformedVertices(std::vector<glm::vec3> vertices, glm::vec3 Position, BlockType type)
{
	std::vector<glm::vec3> newVerts;

	for (glm::vec3 vert : vertices)
	{
        if(type == CraftMine::BlockType::WATER || type == CraftMine::BlockType::LAVA)
        {
            vert.y -= 0.2f;

        }
        newVerts.push_back(vert + Position);
	}
	return newVerts;
}
bool Block::transparent(int id)
{
    if(std::find(transparentBlocks.begin(), transparentBlocks.end(), id) == transparentBlocks.end())
    {
        return false;
    }
    return true;
}
bool Block::isSolid(int id){
    if(std::find(nonSolidBlocks.begin(), nonSolidBlocks.end(), id) != nonSolidBlocks.end())
    {
        return false;
    }
    return true;
}

bool Block::hasCustomMesh(unsigned char id){
    return customMeshDataMap.find(id) != customMeshDataMap.end();
}
