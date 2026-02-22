#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "BlockData.h"
#include <iostream>
#include <algorithm>

using namespace CraftMine;
class Block
{
public:
	static FaceData getFace(Faces face, BlockType type, glm::vec3 position);
	static std::vector<glm::vec3> addTransformedVertices(const std::vector<glm::vec3>& vertices, glm::vec3 Position, BlockType type);
    static std::vector<glm::vec3>
    addOutlineVertices(const std::vector<glm::vec3>& vertices, glm::vec3 Position, bool hasCustomMesh);
    static bool isTransparent(int id);
    static bool isSolid(int id);

    static bool hasCustomMesh(unsigned char id);

    static std::vector<glm::vec3> getOutline(glm::vec3 position, unsigned char id);
};

