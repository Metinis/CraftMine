#include "BlockData.h"

namespace CraftMine {
	const std::map<Faces, std::vector<glm::vec3>> FaceDataRaw::rawVertexData{
			{FRONT, {glm::vec3(-0.5f, 0.5f, 0.5f), // topleft vert
					 glm::vec3(0.5f, 0.5f, 0.5f),  // topright vert
					 glm::vec3(0.5f, -0.5f, 0.5f), // bottomright vert
					 glm::vec3(-0.5f, -0.5f, 0.5f)}},// bottomleft vert

			{RIGHT, {glm::vec3(0.5f, 0.5f, 0.5f),  // topleft vert
					 glm::vec3(0.5f, 0.5f, -0.5f), // topright vert
					 glm::vec3(0.5f, -0.5f, -0.5f),// bottomright vert
					 glm::vec3(0.5f, -0.5f, 0.5f)}}, // bottomleft vert

			{BACK,  {glm::vec3(0.5f, 0.5f, -0.5f),  // topleft vert
					 glm::vec3(-0.5f, 0.5f, -0.5f), // topright vert
					 glm::vec3(-0.5f, -0.5f, -0.5f),// bottomright vert
					 glm::vec3(0.5f, -0.5f, -0.5f)}}, // bottomleft vert

			{LEFT,  {glm::vec3(-0.5f, 0.5f, -0.5f), // topleft vert
					 glm::vec3(-0.5f, 0.5f, 0.5f),  // topright vert
					 glm::vec3(-0.5f, -0.5f, 0.5f), // bottomright vert
					 glm::vec3(-0.5f, -0.5f, -0.5f)}},// bottomleft vert

			{TOP,   {glm::vec3(-0.5f, 0.5f, -0.5f), // topleft vert
					 glm::vec3(0.5f, 0.5f, -0.5f),  // topright vert
					 glm::vec3(0.5f, 0.5f, 0.5f),   // bottomright vert
					 glm::vec3(-0.5f, 0.5f, 0.5f)}},  // bottomleft vert

			{BOTTOM,{glm::vec3(-0.5f, -0.5f, 0.5f), // topleft vert
					 glm::vec3(0.5f, -0.5f, 0.5f),  // topright vert
					 glm::vec3(0.5f, -0.5f, -0.5f), // bottomright vert
					 glm::vec3(-0.5f, -0.5f, -0.5f)}} // bottomleft vert
	};
    glm::vec3 rotateVertex(const glm::vec3& vertex, const float angle) {
        const glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::vec4 tempVertex = rotationMatrix * glm::vec4(vertex, 1.0f);
        return tempVertex;
    }

// Raw vertex data for the flower with rotated faces
    const std::map<Faces, std::vector<glm::vec3>> FaceDataRaw::rawVertexDataFlower{
            {FRONT, {
                            rotateVertex(glm::vec3(-0.5f, 0.5f, 0.0f), 45.0f),
                            rotateVertex(glm::vec3(0.5f, 0.5f, 0.0f), 45.0f),
                            rotateVertex(glm::vec3(0.5f, -0.5f, 0.0f), 45.0f),
                            rotateVertex(glm::vec3(-0.5f, -0.5f, 0.0f), 45.0f)
                    }},
            {BACK, {
                            rotateVertex(glm::vec3(0.0f, 0.5f, -0.5f), 45.0f),
                            rotateVertex(glm::vec3(0.0f, 0.5f, 0.5f), 45.0f),
                            rotateVertex(glm::vec3(0.0f, -0.5f, 0.5f), 45.0f),
                            rotateVertex(glm::vec3(0.0f, -0.5f, -0.5f), 45.0f)
                    }},
    		{RIGHT, {
                            rotateVertex(glm::vec3(-0.5f, 0.5f, 0.0f), -135.0f),
                            rotateVertex(glm::vec3(0.5f, 0.5f, 0.0f), -135.0f),
                            rotateVertex(glm::vec3(0.5f, -0.5f, 0.0f), -135.0f),
                            rotateVertex(glm::vec3(-0.5f, -0.5f, 0.0f), -135.0f)
                    }},
    		{LEFT, {
                            rotateVertex(glm::vec3(-0.5f, 0.5f, 0.0f), -225.0f),
                            rotateVertex(glm::vec3(0.5f, 0.5f, 0.0f), -225.0f),
                            rotateVertex(glm::vec3(0.5f, -0.5f, 0.0f), -225.0f),
                            rotateVertex(glm::vec3(-0.5f, -0.5f, 0.0f), -225.0f)
                    }}
    };

    std::map<unsigned char, BlockType> BlockIDMap{
            {0, EMPTY},
            {1, GRASS},
            {2, DIRT},
            {3, STONE},
            {4, BEDROCK},
            {5, WATER},
            {6, SAND},
            {7, OAK_WOOD},
            {8, OAK_LEAF},
            {9, GLASS},
            {10, WOOD_PLANKS},
            {11, SNOW_GRASS},
            {12, POLISHED_STONE},
            {13, BRICK},
            {14, TNT},
            {15, COBBLESTONE},
            {16, GRAVEL},
            {17, IRON_BLOCK},
            {18, GOLD_BLOCK},
            {19, DIAMOND_BLOCK},
            {20, CHEST},
            {21, GOLD_ORE},
            {22, IRON_ORE},
            {23, COAL_ORE},
            {24, BOOKSHELF},
            {25, MOSSY_COBBLESTONE},
            {26, OBSIDIAN},
            {27, CRAFTING_TABLE},
            {28, FURNACE},
            {29, DISPENSER},
            {30, SPONGE},
            {31, DIAMOND_ORE},
            {32, REDSTONE_ORE},
            {33, STONE_BRICKS},
            {34, WHITE_WOOL},
            {35, SNOW_BLOCK},
            {36, ICE_BLOCK},
            {37, CLAY_BLOCK},
            {38, JUKE_BOX},
            {39, MOSSY_STONE_BRICK},
            {40, CRUMBLED_STONE_BRICK},
            {41, PUMPKIN},
            {42, JACKOLATERN},
            {43, NETHERRACK},
            {44, SOULSAND},
            {45, GLOWSTONE},
            {46, DARK_WOOD},
            {47, BIRCH_WOOD},
            {48, BLACK_WOOL},
            {49, DARK_GREY_WOOL},
            {50, RED_WOOL},
            {51, PINK_WOOL},
            {52, GREEN_WOOL},
            {53, LIME_WOOL},
            {54, BROWN_WOOL},
            {55, YELLOW_WOOL},
            {56, BLUE_WOOL},
            {57, LIGHT_BLUE_WOOL},
            {58, PURLE_WOOL},
            {59, BRIGHT_PURPLE_WOOL},
            {60, CYAN_WOOL},
            {61, ORANGE_WOOL},
            {62, GREY_WOOL},
            {63, LAPIS_BLOCK},
            {64, LAPIS_ORE},
            {65, SANDSTONE},
            {66, NETHERBRICK},
            {67, LAVA},
            {68, DARK_WOOD_LEAVES},
            {69, WATERMELON},
            {70, ROSE_FLOWER},
            {71, DANDELION_FLOWER}
    };
    std::vector<int> transparentBlocks{
        0,
        5,
        8,
        9,
        68,
        67,
        36,
        70,
        71
    };
    std::vector<int> nonSolidBlocks{
            0,
            5,
            67,
            70,
            71
    };
    std::map<Faces, float> brightnessMap{
            {LEFT, 0.80f},
            {RIGHT, 0.80f},
            {TOP, 1.0f},
            {BOTTOM, 1.0f},
            {FRONT, 0.86f},
            {BACK, 0.86f}
    };

    std::map<unsigned char, std::map<Faces, std::vector<glm::vec3>>> customMeshDataMap{
            {70, FaceDataRaw::rawVertexDataFlower},
            {71, FaceDataRaw::rawVertexDataFlower}
    };

    std::map<Faces, std::vector<glm::vec3>> normalsMap{
            {FRONT, {glm::vec3(0.0f, 0.0f, 1.0f), // topleft vert
                            glm::vec3(0.0f, 0.0f, 1.0f),  // topright vert
                            glm::vec3(0.0f, 0.0f, 1.0f), // bottomright vert
                            glm::vec3(0.0f, 0.0f, 1.0f)}},// bottomleft vert

            {RIGHT, {glm::vec3(1.0f, 0.0f, 0.0f), // topleft vert
                            glm::vec3(1.0f, 0.0f, 0.0f),  // topright vert
                            glm::vec3(1.0f, 0.0f, 0.0f), // bottomright vert
                            glm::vec3(1.0f, 0.0f, 0.0f)}},// bottomleft vert

            {BACK,  {glm::vec3(0.0f, 0.0f, -1.0f), // topleft vert
                            glm::vec3(0.0f, 0.0f, -1.0f),  // topright vert
                            glm::vec3(0.0f, 0.0f, -1.0f), // bottomright vert
                            glm::vec3(0.0f, 0.0f, -1.0f)}},// bottomleft vert

            {LEFT,  {glm::vec3(-1.0f, 0.0f, 0.0f), // topleft vert
                            glm::vec3(-1.0f, 0.0f, 0.0f),  // topright vert
                            glm::vec3(-1.0f, 0.0f, 0.0f), // bottomright vert
                            glm::vec3(-1.0f, 0.0f, 0.0f)}},// bottomleft vert

            {TOP,   {glm::vec3(0.0f, 1.0f, 0.0f), // topleft vert
                            glm::vec3(0.0f, 1.0f, 0.0f),  // topright vert
                            glm::vec3(0.0f, 1.0f, 0.0f), // bottomright vert
                            glm::vec3(0.0f, 1.0f, 0.0f)}},// bottomleft vert

            {BOTTOM,{glm::vec3(0.0f, -1.0f, 0.0f), // topleft vert
                            glm::vec3(0.0f, -1.0f, 0.0f),  // topright vert
                            glm::vec3(0.0f, -1.0f, 0.0f), // bottomright vert
                            glm::vec3(0.0f, -1.0f, 0.0f)}},// bottomleft vert
    };
    BlocksToBeAdded::BlocksToBeAdded(const glm::ivec2& chunkPos, const glm::ivec3& localPos, unsigned char id)
            : chunkPosition(chunkPos), localPosition(localPos), blockID(id) {
        // Implementation
    }
}
