//
// Created by vodkasas on 03/02/24.
//

#ifndef CRAFTMINE_GAME_H
#define CRAFTMINE_GAME_H
#pragma once

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <vector>

#include "Input/Camera.h"
#include "Chunk.h"
#include "World.h"
#include "UI.h"
#include "Input/MouseInput.h"

#define SCR_WIDTH 1280
#define SCR_HEIGHT 720



class Game {
public:
    Game();

    void run();

private:
    UI* ui;
    GLFWwindow* window;
    Camera* camera;
    World* world;
    MouseInput* mouseInput;
    glm::ivec2 lastChunkPos{};
    glm::ivec2 newChunkPos{};
    bool wireframe;
    bool keyProcessed;
    float deltaTime;
    float lastFrame;
    int updateingInt;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    static void processInput(GLFWwindow* window, bool* wireframe, bool* keyProccessed, Camera& camera, World& world, float& deltaTime);


};



#endif //CRAFTMINE_GAME_H
