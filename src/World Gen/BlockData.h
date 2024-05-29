#pragma once
#include<GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <map>

class Chunk;
namespace CraftMine {	//using namespace since it conflicts with some cameraMovement stuff
	enum Faces {
		LEFT,
        RIGHT,
        FRONT,
        BACK,
        TOP,
        BOTTOM
	};
	enum BlockType {
		EMPTY,
		GRASS,
		DIRT,
		STONE,
		BEDROCK,
		WATER,
        SAND,
        OAK_WOOD,
        OAK_LEAF,
        GLASS,
        WOOD_PLANKS
	};
    enum Biome {
        MOUNTAIN,
        PLAINS,
        FOREST
    };
    extern std::vector<int> transparentBlocks;
    extern std::vector<int> nonSolidBlocks;
	extern std::map<unsigned char, BlockType> BlockIDMap;
    extern std::map<Faces, float> brightnessMap;
    extern std::map<Faces, std::vector<glm::vec3>> normalsMap;
	struct FaceData
	{
	public:
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> texCoords;
        float brightness = 1.0f; //default brightness
        std::vector<glm::vec3> normals;
	};
	struct FaceDataRaw
	{
	public:
		static const std::map<Faces, std::vector<glm::vec3>> rawVertexData;
	};
    struct BlocksToBeAdded{
        glm::ivec2 chunkPosition;
        glm::ivec3 localPosition;
        unsigned char blockID;

        BlocksToBeAdded(const glm::ivec2& chunkPos, const glm::ivec3& localPos, unsigned char id);
    };
}
