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
#include "WorldGen/Chunk.h"
#include "WorldGen/World.h"
#include "Player/Crosshair.h"
#include "Input/Input.h"
#include "Player/Player.h"
#include "Graphics/ScreenQuad.h"
#include "SceneRenderer.h"

class SceneRenderer;

class Input;

class Game {
public:
    Game();

    void run();

    float deltaTime = 0;
    Player* player;
    Camera* camera;
    World* world;
    Input* mouseInput;
    SceneRenderer* scene;
    static int currentWidth;
    static int currentHeight;
private:
    GLFWwindow* window;
    glm::ivec2 lastChunkPos{};
    glm::ivec2 newChunkPos{};
    bool wireframe;
    bool keyProcessed;
    bool isFullscreen;
    float lastFrame = 0;
    int updateingInt;
    float tickSpeed = 0.005f;
    float currentTime = 0.0f; // Current time in seconds
    double accumulator = 0.0;
    float timeStep = 1.0 / 60; // Fixed time step
    const int TICKS_PER_SECOND = 5;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

};



#endif //CRAFTMINE_GAME_H
