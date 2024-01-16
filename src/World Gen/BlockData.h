#pragma once
#include<GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <map>
namespace CraftMine {	//using namespace since it conflicts with some cameraMovement stuff
	enum Faces {
		FRONT,
		RIGHT,
		BACK,
		LEFT,
		TOP,
		BOTTOM
	};
	enum BlockType {
		EMPTY,
		GRASS,
		DIRT,
		STONE,
		BEDROCK,
		WATER
	};
	extern std::map<unsigned char, BlockType> BlockIDMap;
	struct FaceData
	{
	public:
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> texCoords;
	};
	struct FaceDataRaw
	{
	public:
		static const std::map<Faces, std::vector<glm::vec3>> rawVertexData;

	};
}
