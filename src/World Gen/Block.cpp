#include "Block.h"
#include "TextureData.h"

FaceData Block::GetFace(Faces face, BlockType type, glm::vec3 position)
{
    if(type == CraftMine::EMPTY)
    {
        std::cout<<"empty block \n";
    }
	return FaceData{ AddTransformedVertices(FaceDataRaw::rawVertexData.at(face), position, type), TextureData::GetBlockTypeUV(type, face),
                     CraftMine::brightnessMap.at(face), CraftMine::normalsMap.at(face) };
}
std::vector<glm::vec3> Block::GetOutline(glm::vec3 position)
{
    std::vector<glm::vec3> newVertices;
    for(int i = 0; i < 6; i++)
    {
        Faces currentFace = static_cast<Faces>(i);
        std::vector<glm::vec3> vertices = AddOutlineVertices(FaceDataRaw::rawVertexData.at(currentFace), position);
        newVertices.insert(newVertices.end(),vertices.begin(), vertices.end());
    }
    return newVertices;
}
std::vector<glm::vec3> Block::AddOutlineVertices(std::vector<glm::vec3> vertices, glm::vec3 Position)
{
    std::vector<glm::vec3> newVerts;

    for (glm::vec3 vert : vertices)
    {
        vert = vert * 1.01f;
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
            vert.y -= 0.1f;

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
