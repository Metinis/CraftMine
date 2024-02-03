#include "Block.h"
#include "TextureData.h"

FaceData Block::GetFace(Faces face, BlockType type, glm::vec3 position)
{
	return FaceData{ AddTransformedVertices(FaceDataRaw::rawVertexData.at(face), position, type), TextureData::GetBlockTypeUV(type, face), CraftMine::brightnessMap.at(face) };
}
std::vector<glm::vec3> Block::AddTransformedVertices(std::vector<glm::vec3> vertices, glm::vec3 Position, BlockType type)
{
	std::vector<glm::vec3> newVerts;

	for (glm::vec3 vert : vertices)
	{
        if(type == CraftMine::BlockType::WATER)
        {
            if(vert.y > 0)
            {
                vert.y -= 0.1f;
            }
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
