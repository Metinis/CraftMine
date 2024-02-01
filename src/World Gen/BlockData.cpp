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
	std::map<unsigned char, BlockType> BlockIDMap{
		{0, EMPTY},
		{1, GRASS },
		{2, DIRT },
		{3, STONE },
		{4, BEDROCK},
		{5, WATER},
        {6, SAND},
        {7, OAK_WOOD},
        {8, OAK_LEAF}
	};
    std::vector<int> transparentBlocks{
        0,
        5,
        8
    };
    std::map<Faces, float> brightnessMap{
            {LEFT, 0.80f},
            {RIGHT, 0.80f},
            {TOP, 1.0f},
            {BOTTOM, 1.0f},
            {FRONT, 0.86f},
            {BACK, 0.86f}
    };
}