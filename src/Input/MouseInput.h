//
// Created by vodkasas on 03/02/24.
//
#pragma once
#include "Camera.h"
#include "Chunk.h"
#include "World.h"
#include"glfw/include/GLFW/glfw3.h"

#ifndef CRAFTMINE_MOUSEINPUT_H
#define CRAFTMINE_MOUSEINPUT_H

class GLFWwindow;
class World;
class Scene;

class MouseInput {
private:
    World& world;
    Camera& camera;
    Scene& scene;
    float lastX;
    float lastY;
    bool firstMouse;

public:

    MouseInput(Camera& _camera, World& _world, Scene& _scene);

    void processMouse(GLFWwindow* window, double xPosIn, double yPosIn);

    static void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn);

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif //CRAFTMINE_MOUSEINPUT_H
