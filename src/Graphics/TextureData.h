#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <map>
#include "BlockData.h"

class TextureData
{
private:
	static std::vector<glm::vec2> GetUVs(int atlasID);
public:
	static const std::map<CraftMine::BlockType, std::map<CraftMine::Faces, int>> blockIDMap;

	static std::vector<glm::vec2> GetBlockTypeUV(CraftMine::BlockType type, CraftMine::Faces face);
};

