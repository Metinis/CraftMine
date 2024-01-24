#include "Block.h"
#include "TextureData.h"

Block::Block(glm::vec3 position, int id)
{
	Position = position;

	blockType = BlockIDMap[id];

	if (blockType != CraftMine::EMPTY)
	{
		faces = {
			{Faces::FRONT, FaceData{AddTransformedVertices(FaceDataRaw::rawVertexData.at(Faces::FRONT)), TextureData::GetBlockTypeUV(blockType, Faces::FRONT)}},
			{Faces::RIGHT, FaceData{AddTransformedVertices(FaceDataRaw::rawVertexData.at(Faces::RIGHT)), TextureData::GetBlockTypeUV(blockType, Faces::RIGHT)}},
			{Faces::BACK, FaceData{AddTransformedVertices(FaceDataRaw::rawVertexData.at(Faces::BACK)), TextureData::GetBlockTypeUV(blockType, Faces::BACK)}},
			{Faces::LEFT, FaceData{AddTransformedVertices(FaceDataRaw::rawVertexData.at(Faces::LEFT)), TextureData::GetBlockTypeUV(blockType, Faces::LEFT)}},
			{Faces::TOP, FaceData{AddTransformedVertices(FaceDataRaw::rawVertexData.at(Faces::TOP)), TextureData::GetBlockTypeUV(blockType, Faces::TOP)}},
			{Faces::BOTTOM, FaceData{AddTransformedVertices(FaceDataRaw::rawVertexData.at(Faces::BOTTOM)), TextureData::GetBlockTypeUV(blockType, Faces::BOTTOM)}}
		};
	}
}
void Block::SetBlockType(CraftMine::BlockType type)
{
	blockType = type;
}
std::vector<glm::vec3> Block::AddTransformedVertices(std::vector<glm::vec3> vertices)
{
	std::vector<glm::vec3> newVerts;

	for (glm::vec3 vert : vertices)
	{
		newVerts.push_back(vert + Position);
	}
	return newVerts;
}
FaceData Block::GetFace(Faces face)
{
	return faces[face];
}
//static definitions to avoid initialising instance of block -> might need to adjust block class later on
FaceData Block::GetFace(Faces face, BlockType type, glm::vec3 position)
{
	return FaceData{ AddTransformedVertices(FaceDataRaw::rawVertexData.at(face), position), TextureData::GetBlockTypeUV(type, face) };
}
std::vector<glm::vec3> Block::AddTransformedVertices(std::vector<glm::vec3> vertices, glm::vec3 Position)
{
	std::vector<glm::vec3> newVerts;

	for (glm::vec3 vert : vertices)
	{
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
