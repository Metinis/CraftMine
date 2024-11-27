//
// Created by vodkasas on 03/02/24.
//
#pragma once
#include "Camera.h"
#include "Chunk.h"
#include "World.h"
#include"glfw/include/GLFW/glfw3.h"
#include "Game.h"

#ifndef CRAFTMINE_MOUSEINPUT_H
#define CRAFTMINE_MOUSEINPUT_H

class GLFWwindow;
class World;
class Scene;
class Game;

class Input {
private:

    Game& game;
    World& world;
    Camera& camera;
    Scene& scene;
    Player& player;
    float lastX;
    float lastY;
    bool firstMouse;
    bool wireFrame = false;
    bool fullscreen = false;
    bool isCursorLocked = true;

    int windowedXPos{}, windowedYPos{};
    int windowedWidth{}, windowedHeight{};

    double lastPressTime = 0; //used for jumping
    float timeFrame = 0.25f; //used for jumping

// Declare variables to store the previous cursor position
    double previousCursorX = 0.0, previousCursorY = 0.0;
public:

    Input(Camera &_camera, World &_world, Scene &_scene, Player &_player, Game &_game);

    void processMouse(GLFWwindow *window, double xposIn, double yposIn);

    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) const;

    void processKey(int key, int action, GLFWwindow* window);

    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

    static void processInput(GLFWwindow *window, bool *wireframe, bool *keyProccessed, bool *_isFullscreen, Player &player,
                             World &world, const float &deltaTime, Scene &scene);
};

#endif //CRAFTMINE_MOUSEINPUT_H
