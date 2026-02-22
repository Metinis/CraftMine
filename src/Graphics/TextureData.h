#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <map>
#include "WorldGen/BlockData.h"
#include <cmath>

class TextureData
{
private:
	static std::vector<glm::vec2> getUVs(int atlasID);
public:
	static const std::map<CraftMine::BlockType, std::map<CraftMine::Faces, int>> blockIDMap;

	static std::vector<glm::vec2> getBlockTypeUV(CraftMine::BlockType type, CraftMine::Faces face);
};

