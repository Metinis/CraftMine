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
        WOOD_PLANKS,
        SNOW_GRASS,
        POLISHED_STONE,
        BRICK,
        TNT,
        COBBLESTONE,
        GRAVEL,
        IRON_BLOCK,
        GOLD_BLOCK,
        DIAMOND_BLOCK,
        CHEST,
        GOLD_ORE,
        IRON_ORE,
        COAL_ORE,
        BOOKSHELF,
        MOSSY_COBBLESTONE,
        OBSIDIAN,
        CRAFTING_TABLE,
        FURNACE,
        DISPENSER,
        SPONGE,
        DIAMOND_ORE,
        REDSTONE_ORE,
        STONE_BRICKS,
        WHITE_WOOL,
        SNOW_BLOCK,
        ICE_BLOCK,
        CLAY_BLOCK,
        JUKE_BOX,
        MOSSY_STONE_BRICK,
        CRUMBLED_STONE_BRICK,
        PUMPKIN,
        JACKOLATERN,
        NETHERRACK,
        SOULSAND,
        GLOWSTONE,
        DARK_WOOD,
        BIRCH_WOOD,
        BLACK_WOOL,
        DARK_GREY_WOOL,
        RED_WOOL,
        PINK_WOOL,
        GREEN_WOOL,
        LIME_WOOL,
        BROWN_WOOL,
        YELLOW_WOOL,
        BLUE_WOOL,
        LIGHT_BLUE_WOOL,
        PURLE_WOOL,
        BRIGHT_PURPLE_WOOL,
        CYAN_WOOL,
        ORANGE_WOOL,
        GREY_WOOL,
        LAPIS_BLOCK,
        LAPIS_ORE,
        SANDSTONE,
        NETHERBRICK,
        LAVA,
        DARK_WOOD_LEAVES,
        BIRCK_WOOD_LEAVES,
        WATERMELON
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
        BlocksToBeAdded()
                : chunkPosition(0, 0), localPosition(0, 0, 0), blockID(0) {}
    };
}
