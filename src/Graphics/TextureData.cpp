#include "TextureData.h"

const std::map<CraftMine::BlockType, std::map<CraftMine::Faces, int>> TextureData::blockIDMap{	//maps each face in a blocktype to a textureFBO

		{CraftMine::BlockType::GRASS,
			{{CraftMine::Faces::BACK, 3},
			{CraftMine::Faces::FRONT, 3},
			{CraftMine::Faces::LEFT, 3},
			{CraftMine::Faces::RIGHT, 3},
			{CraftMine::Faces::TOP, 0},
			{CraftMine::Faces::BOTTOM, 2}},
		},
		{CraftMine::BlockType::DIRT,
			{{CraftMine::Faces::BACK, 2},
			{CraftMine::Faces::FRONT, 2},
			{CraftMine::Faces::LEFT, 2},
			{CraftMine::Faces::RIGHT, 2},
			{CraftMine::Faces::TOP, 2},
			{CraftMine::Faces::BOTTOM, 2}},
		},
		{CraftMine::BlockType::STONE,
			{{CraftMine::Faces::BACK, 1},
			{CraftMine::Faces::FRONT, 1},
			{CraftMine::Faces::LEFT, 1},
			{CraftMine::Faces::RIGHT, 1},
			{CraftMine::Faces::TOP, 1},
			{CraftMine::Faces::BOTTOM, 1}},
		},
		{CraftMine::BlockType::BEDROCK,
			{{CraftMine::Faces::BACK, 17},
			{CraftMine::Faces::FRONT, 17},
			{CraftMine::Faces::LEFT, 17},
			{CraftMine::Faces::RIGHT, 17},
			{CraftMine::Faces::TOP, 17},
			{CraftMine::Faces::BOTTOM, 17}},
		},
		{CraftMine::BlockType::WATER,
			{{CraftMine::Faces::BACK, 205},
			{CraftMine::Faces::FRONT, 205},
			{CraftMine::Faces::LEFT, 205},
			{CraftMine::Faces::RIGHT, 205},
			{CraftMine::Faces::TOP, 205},
			{CraftMine::Faces::BOTTOM, 205}},
		},
        {CraftMine::BlockType::SAND,
         {{CraftMine::Faces::BACK, 18},
            {CraftMine::Faces::FRONT, 18},
            {CraftMine::Faces::LEFT, 18},
            {CraftMine::Faces::RIGHT, 18},
            {CraftMine::Faces::TOP, 18},
            {CraftMine::Faces::BOTTOM, 18}},
        },
        {CraftMine::BlockType::OAK_WOOD,
         {{CraftMine::Faces::BACK, 20},
          {CraftMine::Faces::FRONT, 20},
          {CraftMine::Faces::LEFT, 20},
          {CraftMine::Faces::RIGHT, 20},
          {CraftMine::Faces::TOP, 21},
          {CraftMine::Faces::BOTTOM, 21}},
        },
        {CraftMine::BlockType::OAK_LEAF,
         {{CraftMine::Faces::BACK, 52},
          {CraftMine::Faces::FRONT, 52},
          {CraftMine::Faces::LEFT, 52},
          {CraftMine::Faces::RIGHT, 52},
          {CraftMine::Faces::TOP, 52},
          {CraftMine::Faces::BOTTOM, 52}},
        },
        {CraftMine::BlockType::GLASS,
         {{CraftMine::Faces::BACK, 49},
          {CraftMine::Faces::FRONT, 49},
          {CraftMine::Faces::LEFT, 49},
          {CraftMine::Faces::RIGHT, 49},
          {CraftMine::Faces::TOP, 49},
          {CraftMine::Faces::BOTTOM, 49}},
        },
        {CraftMine::BlockType::WOOD_PLANKS,
         {{CraftMine::Faces::BACK, 4},
          {CraftMine::Faces::FRONT, 4},
          {CraftMine::Faces::LEFT, 4},
          {CraftMine::Faces::RIGHT, 4},
          {CraftMine::Faces::TOP, 4},
          {CraftMine::Faces::BOTTOM, 4}},
        },
        {CraftMine::BlockType::SNOW_GRASS,
         {{CraftMine::Faces::BACK, 68},
          {CraftMine::Faces::FRONT, 68},
          {CraftMine::Faces::LEFT, 68},
          {CraftMine::Faces::RIGHT, 68},
          {CraftMine::Faces::TOP, 66},
          {CraftMine::Faces::BOTTOM, 2}},
        },
};
std::vector<glm::vec2> TextureData::GetUVs(int atlasID)
{
	int row = (atlasID / 16) + 1;
    float left = (float)atlasID / 16.0f;
    float right = ((float)atlasID + 1.0f) / 16.0f;
    float top = row / 16.0f;
    float bottom = (row - 1.0f) / 16.0f;
	std::vector<glm::vec2> uvs = {
		glm::vec2(left, bottom),  //bottom left
		glm::vec2(right, bottom),   //bottom right
		glm::vec2(right, top),  //top right
		glm::vec2(left, top),			//top left
	};
	return uvs;
}
std::vector<glm::vec2> TextureData::GetBlockTypeUV(CraftMine::BlockType type, CraftMine::Faces face)
{
	std::map<CraftMine::Faces, int> faceMap = blockIDMap.at(type);

	return GetUVs(faceMap[face]);
}