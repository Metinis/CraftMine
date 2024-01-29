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
private:
	std::map<Faces, FaceData> faces;
	std::vector<glm::vec3> AddTransformedVertices(std::vector<glm::vec3> vertices);
public:
	//encapsulate when you can
	CraftMine::BlockType blockType = CraftMine::BlockType::EMPTY; 
	glm::vec3 Position;

    Block(glm::vec3 position, int id);

	void SetBlockType(CraftMine::BlockType type);

	FaceData GetFace(Faces face);

	static FaceData GetFace(Faces face, BlockType type, glm::vec3 position);
	static std::vector<glm::vec3> AddTransformedVertices(std::vector<glm::vec3> vertices, glm::vec3 Position, BlockType type);
    static bool transparent(int id);

};

