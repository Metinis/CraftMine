#include "Game.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char* argv[])
{
    std::string username = "Player";
    if (argc > 1) {
        username = argv[1];
    }
    Game game(username);
    game.run();

  	return 0;

}

