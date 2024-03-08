#pragma once
#include<GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <map>
#include "BlockData.h"
#include <iostream>
#include <algorithm>

using namespace CraftMine;
class Block
{
public:
	static FaceData GetFace(Faces face, BlockType type, glm::vec3 position);
	static std::vector<glm::vec3> AddTransformedVertices(std::vector<glm::vec3> vertices, glm::vec3 Position, BlockType type);
    static std::vector<glm::vec3> GetOutline(glm::vec3 position);
    static std::vector<glm::vec3> AddOutlineVertices(std::vector<glm::vec3> vertices, glm::vec3 Position);
    static bool transparent(int id);
    static bool isSolid(int id);
};

