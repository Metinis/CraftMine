#include "TextureData.h"

const std::map<CraftMine::BlockType, std::map<CraftMine::Faces, int>> TextureData::blockIDMap{	//maps each face in a blocktype to a textureFBO

		{CraftMine::BlockType::GRASS,
			{{CraftMine::Faces::BACK, 3},
			{CraftMine::Faces::FRONT, 3},
			{CraftMine::Faces::LEFT, 3},
			{CraftMine::Faces::RIGHT, 3},
			{CraftMine::Faces::TOP, 0},
			{CraftMine::Faces::BOTTOM, 2}},
		},
		{CraftMine::BlockType::DIRT,
			{{CraftMine::Faces::BACK, 2},
			{CraftMine::Faces::FRONT, 2},
			{CraftMine::Faces::LEFT, 2},
			{CraftMine::Faces::RIGHT, 2},
			{CraftMine::Faces::TOP, 2},
			{CraftMine::Faces::BOTTOM, 2}},
		},
		{CraftMine::BlockType::STONE,
			{{CraftMine::Faces::BACK, 1},
			{CraftMine::Faces::FRONT, 1},
			{CraftMine::Faces::LEFT, 1},
			{CraftMine::Faces::RIGHT, 1},
			{CraftMine::Faces::TOP, 1},
			{CraftMine::Faces::BOTTOM, 1}},
		},
		{CraftMine::BlockType::BEDROCK,
			{{CraftMine::Faces::BACK, 17},
			{CraftMine::Faces::FRONT, 17},
			{CraftMine::Faces::LEFT, 17},
			{CraftMine::Faces::RIGHT, 17},
			{CraftMine::Faces::TOP, 17},
			{CraftMine::Faces::BOTTOM, 17}},
		},
		{CraftMine::BlockType::WATER,
			{{CraftMine::Faces::BACK, 205},
			{CraftMine::Faces::FRONT, 205},
			{CraftMine::Faces::LEFT, 205},
			{CraftMine::Faces::RIGHT, 205},
			{CraftMine::Faces::TOP, 205},
			{CraftMine::Faces::BOTTOM, 205}},
		},
        {CraftMine::BlockType::SAND,
         {{CraftMine::Faces::BACK, 18},
            {CraftMine::Faces::FRONT, 18},
            {CraftMine::Faces::LEFT, 18},
            {CraftMine::Faces::RIGHT, 18},
            {CraftMine::Faces::TOP, 18},
            {CraftMine::Faces::BOTTOM, 18}},
        },
        {CraftMine::BlockType::OAK_WOOD,
         {{CraftMine::Faces::BACK, 20},
          {CraftMine::Faces::FRONT, 20},
          {CraftMine::Faces::LEFT, 20},
          {CraftMine::Faces::RIGHT, 20},
          {CraftMine::Faces::TOP, 21},
          {CraftMine::Faces::BOTTOM, 21}},
        },
        {CraftMine::BlockType::OAK_LEAF,
         {{CraftMine::Faces::BACK, 52},
          {CraftMine::Faces::FRONT, 52},
          {CraftMine::Faces::LEFT, 52},
          {CraftMine::Faces::RIGHT, 52},
          {CraftMine::Faces::TOP, 52},
          {CraftMine::Faces::BOTTOM, 52}},
        },
        {CraftMine::BlockType::GLASS,
         {{CraftMine::Faces::BACK, 49},
          {CraftMine::Faces::FRONT, 49},
          {CraftMine::Faces::LEFT, 49},
          {CraftMine::Faces::RIGHT, 49},
          {CraftMine::Faces::TOP, 49},
          {CraftMine::Faces::BOTTOM, 49}},
        },
        {CraftMine::BlockType::WOOD_PLANKS,
         {{CraftMine::Faces::BACK, 4},
          {CraftMine::Faces::FRONT, 4},
          {CraftMine::Faces::LEFT, 4},
          {CraftMine::Faces::RIGHT, 4},
          {CraftMine::Faces::TOP, 4},
          {CraftMine::Faces::BOTTOM, 4}},
        },
        {CraftMine::BlockType::SNOW_GRASS,
         {{CraftMine::Faces::BACK, 68},
          {CraftMine::Faces::FRONT, 68},
          {CraftMine::Faces::LEFT, 68},
          {CraftMine::Faces::RIGHT, 68},
          {CraftMine::Faces::TOP, 66},
          {CraftMine::Faces::BOTTOM, 2}},
        },
        {CraftMine::BlockType::POLISHED_STONE,
         {{CraftMine::Faces::BACK, 5},
          {CraftMine::Faces::FRONT, 5},
          {CraftMine::Faces::LEFT, 5},
          {CraftMine::Faces::RIGHT, 5},
          {CraftMine::Faces::TOP, 6},
          {CraftMine::Faces::BOTTOM, 6}},
        },
        {CraftMine::BlockType::BRICK,
         {{CraftMine::Faces::BACK, 7},
          {CraftMine::Faces::FRONT, 7},
          {CraftMine::Faces::LEFT, 7},
          {CraftMine::Faces::RIGHT, 7},
          {CraftMine::Faces::TOP, 7},
          {CraftMine::Faces::BOTTOM, 7}},
        },
        {CraftMine::BlockType::TNT,
         {{CraftMine::Faces::BACK, 8},
          {CraftMine::Faces::FRONT, 8},
          {CraftMine::Faces::LEFT, 8},
          {CraftMine::Faces::RIGHT, 8},
          {CraftMine::Faces::TOP, 9},
          {CraftMine::Faces::BOTTOM, 10}},
        },
        {CraftMine::BlockType::COBBLESTONE,
         {{CraftMine::Faces::BACK, 16},
          {CraftMine::Faces::FRONT, 16},
          {CraftMine::Faces::LEFT, 16},
          {CraftMine::Faces::RIGHT, 16},
          {CraftMine::Faces::TOP, 16},
          {CraftMine::Faces::BOTTOM, 16}},
        },
        {CraftMine::BlockType::GRAVEL,
         {{CraftMine::Faces::BACK, 19},
          {CraftMine::Faces::FRONT, 19},
          {CraftMine::Faces::LEFT, 19},
          {CraftMine::Faces::RIGHT, 19},
          {CraftMine::Faces::TOP, 19},
          {CraftMine::Faces::BOTTOM, 19}},
        },
        {CraftMine::BlockType::IRON_BLOCK,
         {{CraftMine::Faces::BACK, 22},
          {CraftMine::Faces::FRONT, 22},
          {CraftMine::Faces::LEFT, 22},
          {CraftMine::Faces::RIGHT, 22},
          {CraftMine::Faces::TOP, 22},
          {CraftMine::Faces::BOTTOM, 22}},
        },
        {CraftMine::BlockType::GOLD_BLOCK,
         {{CraftMine::Faces::BACK, 23},
          {CraftMine::Faces::FRONT, 23},
          {CraftMine::Faces::LEFT, 23},
          {CraftMine::Faces::RIGHT, 23},
          {CraftMine::Faces::TOP, 23},
          {CraftMine::Faces::BOTTOM, 23}},
        },
        {CraftMine::BlockType::DIAMOND_BLOCK,
         {{CraftMine::Faces::BACK, 24},
          {CraftMine::Faces::FRONT, 24},
          {CraftMine::Faces::LEFT, 24},
          {CraftMine::Faces::RIGHT, 24},
          {CraftMine::Faces::TOP, 24},
          {CraftMine::Faces::BOTTOM, 24}},
        },
        {CraftMine::BlockType::CHEST,
         {{CraftMine::Faces::BACK, 26},
          {CraftMine::Faces::FRONT, 27},
          {CraftMine::Faces::LEFT, 26},
          {CraftMine::Faces::RIGHT, 26},
          {CraftMine::Faces::TOP, 25},
          {CraftMine::Faces::BOTTOM, 25}},
        },
        {CraftMine::BlockType::GOLD_ORE,
         {{CraftMine::Faces::BACK, 32},
          {CraftMine::Faces::FRONT, 32},
          {CraftMine::Faces::LEFT, 32},
          {CraftMine::Faces::RIGHT, 32},
          {CraftMine::Faces::TOP, 32},
          {CraftMine::Faces::BOTTOM, 32}},
        },
        {CraftMine::BlockType::IRON_ORE,
         {{CraftMine::Faces::BACK, 33},
          {CraftMine::Faces::FRONT, 33},
          {CraftMine::Faces::LEFT, 33},
          {CraftMine::Faces::RIGHT, 33},
          {CraftMine::Faces::TOP, 33},
          {CraftMine::Faces::BOTTOM, 33}},
        },
        {CraftMine::BlockType::COAL_ORE,
         {{CraftMine::Faces::BACK, 34},
          {CraftMine::Faces::FRONT, 34},
          {CraftMine::Faces::LEFT, 34},
          {CraftMine::Faces::RIGHT, 34},
          {CraftMine::Faces::TOP, 34},
          {CraftMine::Faces::BOTTOM, 34}},
        },
        {CraftMine::BlockType::BOOKSHELF,
         {{CraftMine::Faces::BACK, 35},
          {CraftMine::Faces::FRONT, 35},
          {CraftMine::Faces::LEFT, 35},
          {CraftMine::Faces::RIGHT, 35},
          {CraftMine::Faces::TOP, 4},
          {CraftMine::Faces::BOTTOM, 4}},
        },
        {CraftMine::BlockType::MOSSY_COBBLESTONE,
         {{CraftMine::Faces::BACK, 36},
          {CraftMine::Faces::FRONT, 36},
          {CraftMine::Faces::LEFT, 36},
          {CraftMine::Faces::RIGHT, 36},
          {CraftMine::Faces::TOP, 36},
          {CraftMine::Faces::BOTTOM, 36}},
        },
        {CraftMine::BlockType::OBSIDIAN,
         {{CraftMine::Faces::BACK, 37},
          {CraftMine::Faces::FRONT, 37},
          {CraftMine::Faces::LEFT, 37},
          {CraftMine::Faces::RIGHT, 37},
          {CraftMine::Faces::TOP, 37},
          {CraftMine::Faces::BOTTOM, 37}},
        },
        {CraftMine::BlockType::CRAFTING_TABLE,
         {{CraftMine::Faces::BACK, 59},
          {CraftMine::Faces::FRONT, 59},
          {CraftMine::Faces::LEFT, 60},
          {CraftMine::Faces::RIGHT, 60},
          {CraftMine::Faces::TOP, 43},
          {CraftMine::Faces::BOTTOM, 4}},
        },
        {CraftMine::BlockType::FURNACE,
         {{CraftMine::Faces::BACK, 45},
          {CraftMine::Faces::FRONT, 44},
          {CraftMine::Faces::LEFT, 45},
          {CraftMine::Faces::RIGHT, 45},
          {CraftMine::Faces::TOP, 1},
          {CraftMine::Faces::BOTTOM, 1}},
        },
        {CraftMine::BlockType::DISPENSER,
         {{CraftMine::Faces::BACK, 45},
          {CraftMine::Faces::FRONT, 46},
          {CraftMine::Faces::LEFT, 45},
          {CraftMine::Faces::RIGHT, 45},
          {CraftMine::Faces::TOP, 1},
          {CraftMine::Faces::BOTTOM, 1}},
        },
        {CraftMine::BlockType::SPONGE,
         {{CraftMine::Faces::BACK, 48},
          {CraftMine::Faces::FRONT, 48},
          {CraftMine::Faces::LEFT, 48},
          {CraftMine::Faces::RIGHT, 48},
          {CraftMine::Faces::TOP, 48},
          {CraftMine::Faces::BOTTOM, 48}},
        },
        {CraftMine::BlockType::DIAMOND_ORE,
         {{CraftMine::Faces::BACK, 50},
          {CraftMine::Faces::FRONT, 50},
          {CraftMine::Faces::LEFT, 50},
          {CraftMine::Faces::RIGHT, 50},
          {CraftMine::Faces::TOP, 50},
          {CraftMine::Faces::BOTTOM, 50}},
        },
        {CraftMine::BlockType::REDSTONE_ORE,
         {{CraftMine::Faces::BACK, 51},
          {CraftMine::Faces::FRONT, 51},
          {CraftMine::Faces::LEFT, 51},
          {CraftMine::Faces::RIGHT, 51},
          {CraftMine::Faces::TOP, 51},
          {CraftMine::Faces::BOTTOM, 51}},
        },
        {CraftMine::BlockType::STONE_BRICKS,
         {{CraftMine::Faces::BACK, 54},
          {CraftMine::Faces::FRONT, 54},
          {CraftMine::Faces::LEFT, 54},
          {CraftMine::Faces::RIGHT, 54},
          {CraftMine::Faces::TOP, 54},
          {CraftMine::Faces::BOTTOM, 54}},
        },
        {CraftMine::BlockType::WHITE_WOOL,
         {{CraftMine::Faces::BACK, 64},
          {CraftMine::Faces::FRONT, 64},
          {CraftMine::Faces::LEFT, 64},
          {CraftMine::Faces::RIGHT, 64},
          {CraftMine::Faces::TOP, 64},
          {CraftMine::Faces::BOTTOM, 64}},
        },
        {CraftMine::BlockType::SNOW_BLOCK,
         {{CraftMine::Faces::BACK, 66},
          {CraftMine::Faces::FRONT, 66},
          {CraftMine::Faces::LEFT, 66},
          {CraftMine::Faces::RIGHT, 66},
          {CraftMine::Faces::TOP, 66},
          {CraftMine::Faces::BOTTOM, 66}},
        },
        {CraftMine::BlockType::ICE_BLOCK,
         {{CraftMine::Faces::BACK, 67},
          {CraftMine::Faces::FRONT, 67},
          {CraftMine::Faces::LEFT, 67},
          {CraftMine::Faces::RIGHT, 67},
          {CraftMine::Faces::TOP, 67},
          {CraftMine::Faces::BOTTOM, 67}},
        },
        {CraftMine::BlockType::CLAY_BLOCK,
         {{CraftMine::Faces::BACK, 72},
          {CraftMine::Faces::FRONT, 72},
          {CraftMine::Faces::LEFT, 72},
          {CraftMine::Faces::RIGHT, 72},
          {CraftMine::Faces::TOP, 72},
          {CraftMine::Faces::BOTTOM, 72}},
        },
        {CraftMine::BlockType::JUKE_BOX,
         {{CraftMine::Faces::BACK, 74},
          {CraftMine::Faces::FRONT, 74},
          {CraftMine::Faces::LEFT, 74},
          {CraftMine::Faces::RIGHT, 74},
          {CraftMine::Faces::TOP, 75},
          {CraftMine::Faces::BOTTOM, 74}},
        },
        {CraftMine::BlockType::MOSSY_STONE_BRICK,
         {{CraftMine::Faces::BACK, 100},
          {CraftMine::Faces::FRONT, 100},
          {CraftMine::Faces::LEFT, 100},
          {CraftMine::Faces::RIGHT, 100},
          {CraftMine::Faces::TOP, 100},
          {CraftMine::Faces::BOTTOM, 100}},
        },
        {CraftMine::BlockType::CRUMBLED_STONE_BRICK,
         {{CraftMine::Faces::BACK, 101},
          {CraftMine::Faces::FRONT, 101},
          {CraftMine::Faces::LEFT, 101},
          {CraftMine::Faces::RIGHT, 101},
          {CraftMine::Faces::TOP, 101},
          {CraftMine::Faces::BOTTOM, 101}},
        },
        {CraftMine::BlockType::PUMPKIN,
         {{CraftMine::Faces::BACK, 118},
          {CraftMine::Faces::FRONT, 119},
          {CraftMine::Faces::LEFT, 118},
          {CraftMine::Faces::RIGHT, 118},
          {CraftMine::Faces::TOP, 102},
          {CraftMine::Faces::BOTTOM, 118}},
        },
        {CraftMine::BlockType::JACKOLATERN,
         {{CraftMine::Faces::BACK, 118},
          {CraftMine::Faces::FRONT, 120},
          {CraftMine::Faces::LEFT, 118},
          {CraftMine::Faces::RIGHT, 118},
          {CraftMine::Faces::TOP, 102},
          {CraftMine::Faces::BOTTOM, 118}},
        },
        {CraftMine::BlockType::NETHERRACK,
         {{CraftMine::Faces::BACK, 103},
          {CraftMine::Faces::FRONT, 103},
          {CraftMine::Faces::LEFT, 103},
          {CraftMine::Faces::RIGHT, 103},
          {CraftMine::Faces::TOP, 103},
          {CraftMine::Faces::BOTTOM, 103}},
        },
        {CraftMine::BlockType::SOULSAND,
         {{CraftMine::Faces::BACK, 104},
          {CraftMine::Faces::FRONT, 104},
          {CraftMine::Faces::LEFT, 104},
          {CraftMine::Faces::RIGHT, 104},
          {CraftMine::Faces::TOP, 104},
          {CraftMine::Faces::BOTTOM, 104}},
        },
        {CraftMine::BlockType::GLOWSTONE,
         {{CraftMine::Faces::BACK, 105},
          {CraftMine::Faces::FRONT, 105},
          {CraftMine::Faces::LEFT, 105},
          {CraftMine::Faces::RIGHT, 105},
          {CraftMine::Faces::TOP, 105},
          {CraftMine::Faces::BOTTOM, 105}},
        },
        {CraftMine::BlockType::DARK_WOOD,
         {{CraftMine::Faces::BACK, 116},
          {CraftMine::Faces::FRONT, 116},
          {CraftMine::Faces::LEFT, 116},
          {CraftMine::Faces::RIGHT, 116},
          {CraftMine::Faces::TOP, 21},
          {CraftMine::Faces::BOTTOM, 21}},
        },
        {CraftMine::BlockType::BIRCH_WOOD,
         {{CraftMine::Faces::BACK, 117},
          {CraftMine::Faces::FRONT, 117},
          {CraftMine::Faces::LEFT, 117},
          {CraftMine::Faces::RIGHT, 117},
          {CraftMine::Faces::TOP, 21},
          {CraftMine::Faces::BOTTOM, 21}},
        },
        {CraftMine::BlockType::BLACK_WOOL,
         {{CraftMine::Faces::BACK, 113},
          {CraftMine::Faces::FRONT, 113},
          {CraftMine::Faces::LEFT, 113},
          {CraftMine::Faces::RIGHT, 113},
          {CraftMine::Faces::TOP, 113},
          {CraftMine::Faces::BOTTOM, 113}},
        },
        {CraftMine::BlockType::DARK_GREY_WOOL,
         {{CraftMine::Faces::BACK, 114},
          {CraftMine::Faces::FRONT, 114},
          {CraftMine::Faces::LEFT, 114},
          {CraftMine::Faces::RIGHT, 114},
          {CraftMine::Faces::TOP, 114},
          {CraftMine::Faces::BOTTOM, 114}},
        },
        {CraftMine::BlockType::RED_WOOL,
         {{CraftMine::Faces::BACK, 129},
          {CraftMine::Faces::FRONT, 129},
          {CraftMine::Faces::LEFT, 129},
          {CraftMine::Faces::RIGHT, 129},
          {CraftMine::Faces::TOP, 129},
          {CraftMine::Faces::BOTTOM, 129}},
        },
        {CraftMine::BlockType::PINK_WOOL,
         {{CraftMine::Faces::BACK, 130},
          {CraftMine::Faces::FRONT, 130},
          {CraftMine::Faces::LEFT, 130},
          {CraftMine::Faces::RIGHT, 130},
          {CraftMine::Faces::TOP, 130},
          {CraftMine::Faces::BOTTOM, 130}},
        },
        {CraftMine::BlockType::GREEN_WOOL,
         {{CraftMine::Faces::BACK, 145},
          {CraftMine::Faces::FRONT, 145},
          {CraftMine::Faces::LEFT, 145},
          {CraftMine::Faces::RIGHT, 145},
          {CraftMine::Faces::TOP, 145},
          {CraftMine::Faces::BOTTOM, 145}},
        },
        {CraftMine::BlockType::LIME_WOOL,
         {{CraftMine::Faces::BACK, 146},
          {CraftMine::Faces::FRONT, 146},
          {CraftMine::Faces::LEFT, 146},
          {CraftMine::Faces::RIGHT, 146},
          {CraftMine::Faces::TOP, 146},
          {CraftMine::Faces::BOTTOM, 146}},
        },
        {CraftMine::BlockType::BROWN_WOOL,
         {{CraftMine::Faces::BACK, 161},
          {CraftMine::Faces::FRONT, 161},
          {CraftMine::Faces::LEFT, 161},
          {CraftMine::Faces::RIGHT, 161},
          {CraftMine::Faces::TOP, 161},
          {CraftMine::Faces::BOTTOM, 161}},
        },
        {CraftMine::BlockType::YELLOW_WOOL,
         {{CraftMine::Faces::BACK, 162},
          {CraftMine::Faces::FRONT, 162},
          {CraftMine::Faces::LEFT, 162},
          {CraftMine::Faces::RIGHT, 162},
          {CraftMine::Faces::TOP, 162},
          {CraftMine::Faces::BOTTOM, 162}},
        },
        {CraftMine::BlockType::BLUE_WOOL,
         {{CraftMine::Faces::BACK, 177},
          {CraftMine::Faces::FRONT, 177},
          {CraftMine::Faces::LEFT, 177},
          {CraftMine::Faces::RIGHT, 177},
          {CraftMine::Faces::TOP, 177},
          {CraftMine::Faces::BOTTOM, 177}},
        },
        {CraftMine::BlockType::LIGHT_BLUE_WOOL,
         {{CraftMine::Faces::BACK, 178},
          {CraftMine::Faces::FRONT, 178},
          {CraftMine::Faces::LEFT, 178},
          {CraftMine::Faces::RIGHT, 178},
          {CraftMine::Faces::TOP, 178},
          {CraftMine::Faces::BOTTOM, 178}},
        },
        {CraftMine::BlockType::PURLE_WOOL,
         {{CraftMine::Faces::BACK, 193},
          {CraftMine::Faces::FRONT, 193},
          {CraftMine::Faces::LEFT, 193},
          {CraftMine::Faces::RIGHT, 193},
          {CraftMine::Faces::TOP, 193},
          {CraftMine::Faces::BOTTOM, 193}},
        },
        {CraftMine::BlockType::BRIGHT_PURPLE_WOOL,
         {{CraftMine::Faces::BACK, 194},
          {CraftMine::Faces::FRONT, 194},
          {CraftMine::Faces::LEFT, 194},
          {CraftMine::Faces::RIGHT, 194},
          {CraftMine::Faces::TOP, 194},
          {CraftMine::Faces::BOTTOM, 194}},
        },
        {CraftMine::BlockType::CYAN_WOOL,
         {{CraftMine::Faces::BACK, 209},
          {CraftMine::Faces::FRONT, 209},
          {CraftMine::Faces::LEFT, 209},
          {CraftMine::Faces::RIGHT, 209},
          {CraftMine::Faces::TOP, 209},
          {CraftMine::Faces::BOTTOM, 209}},
        },
        {CraftMine::BlockType::ORANGE_WOOL,
         {{CraftMine::Faces::BACK, 210},
          {CraftMine::Faces::FRONT, 210},
          {CraftMine::Faces::LEFT, 210},
          {CraftMine::Faces::RIGHT, 210},
          {CraftMine::Faces::TOP, 210},
          {CraftMine::Faces::BOTTOM, 210}},
        },
        {CraftMine::BlockType::GREY_WOOL,
         {{CraftMine::Faces::BACK, 225},
          {CraftMine::Faces::FRONT, 225},
          {CraftMine::Faces::LEFT, 225},
          {CraftMine::Faces::RIGHT, 225},
          {CraftMine::Faces::TOP, 225},
          {CraftMine::Faces::BOTTOM, 225}},
        },
        {CraftMine::BlockType::LAPIS_BLOCK,
         {{CraftMine::Faces::BACK, 144},
          {CraftMine::Faces::FRONT, 144},
          {CraftMine::Faces::LEFT, 144},
          {CraftMine::Faces::RIGHT, 144},
          {CraftMine::Faces::TOP, 144},
          {CraftMine::Faces::BOTTOM, 144}},
        },
        {CraftMine::BlockType::LAPIS_ORE,
         {{CraftMine::Faces::BACK, 160},
          {CraftMine::Faces::FRONT, 160},
          {CraftMine::Faces::LEFT, 160},
          {CraftMine::Faces::RIGHT, 160},
          {CraftMine::Faces::TOP, 160},
          {CraftMine::Faces::BOTTOM, 160}},
        },
        {CraftMine::BlockType::SANDSTONE,
         {{CraftMine::Faces::BACK, 192},
          {CraftMine::Faces::FRONT, 192},
          {CraftMine::Faces::LEFT, 192},
          {CraftMine::Faces::RIGHT, 192},
          {CraftMine::Faces::TOP, 176},
          {CraftMine::Faces::BOTTOM, 208}},
        },
        {CraftMine::BlockType::NETHERBRICK,
         {{CraftMine::Faces::BACK, 224},
          {CraftMine::Faces::FRONT, 224},
          {CraftMine::Faces::LEFT, 224},
          {CraftMine::Faces::RIGHT, 224},
          {CraftMine::Faces::TOP, 224},
          {CraftMine::Faces::BOTTOM, 224}},
        },
        {CraftMine::BlockType::LAVA,
         {{CraftMine::Faces::BACK, 237},
          {CraftMine::Faces::FRONT, 237},
          {CraftMine::Faces::LEFT, 237},
          {CraftMine::Faces::RIGHT, 237},
          {CraftMine::Faces::TOP, 237},
          {CraftMine::Faces::BOTTOM, 237}},
        },
        {CraftMine::BlockType::DARK_WOOD_LEAVES,
         {{CraftMine::Faces::BACK, 132},
          {CraftMine::Faces::FRONT, 132},
          {CraftMine::Faces::LEFT, 132},
          {CraftMine::Faces::RIGHT, 132},
          {CraftMine::Faces::TOP, 132},
          {CraftMine::Faces::BOTTOM, 132}},
        },
        {CraftMine::BlockType::WATERMELON,
         {{CraftMine::Faces::BACK, 136},
          {CraftMine::Faces::FRONT, 136},
          {CraftMine::Faces::LEFT, 136},
          {CraftMine::Faces::RIGHT, 136},
          {CraftMine::Faces::TOP, 136},
          {CraftMine::Faces::BOTTOM, 137}},
        }

};
std::vector<glm::vec2> TextureData::GetUVs(int atlasID)
{
	int row = (atlasID / 16) + 1;
    float left = (float)atlasID / 16.0f;
    float right = ((float)atlasID + 1.0f) / 16.0f;
    float top = row / 16.0f;
    float bottom = (row - 1.0f) / 16.0f;
	std::vector<glm::vec2> uvs = {
		glm::vec2(left, bottom),  //bottom left
		glm::vec2(right, bottom),   //bottom right
		glm::vec2(right, top),  //top right
		glm::vec2(left, top),			//top left
	};
	return uvs;
}
std::vector<glm::vec2> TextureData::GetBlockTypeUV(CraftMine::BlockType type, CraftMine::Faces face)
{
	std::map<CraftMine::Faces, int> faceMap = blockIDMap.at(type);

	return GetUVs(faceMap[face]);
}