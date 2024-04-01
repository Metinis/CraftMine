//
// Created by vodkasas on 03/02/24.
//

#ifndef CRAFTMINE_GAME_H
#define CRAFTMINE_GAME_H
#pragma once

#include<glad/glad.h>
#include<glfw/include/GLFW/glfw3.h>
#include <vector>

#include "Input/Camera.h"
#include "Chunk.h"
#include "World.h"
#include "UI.h"
#include "Input/MouseInput.h"
#include "Player/Player.h"
#include "ScreenQuad.h"
#include "Scene.h"

class Scene;

class MouseInput;

class Game {
public:
    Game();

    void run();

private:
    GLFWwindow* window;
    Player* player;
    Camera* camera;
    World* world;
    MouseInput* mouseInput;
    Scene* scene;
    glm::ivec2 lastChunkPos{};
    glm::ivec2 newChunkPos{};
    bool wireframe;
    bool keyProcessed;
    bool isFullscreen;
    float deltaTime;
    float lastFrame;
    int updateingInt;
    float tickSpeed = 0.002f;
    float currentTime = 0.0f; // Current time in seconds
    float dayDuration = 60.0f; // Total duration of a day in seconds


    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    static void processInput(GLFWwindow* window, bool* wireframe, bool* keyProccessed, bool* _isFullscreen, Player& player, World& world, float& deltaTime);

};



#endif //CRAFTMINE_GAME_H
