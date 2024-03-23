//
// Created by vodkasas on 03/02/24.
//
#pragma once
#include "Camera.h"
#include "Chunk.h"
#include "World.h"
#include"glfw-3.3.9/include/GLFW/glfw3.h"

#ifndef CRAFTMINE_MOUSEINPUT_H
#define CRAFTMINE_MOUSEINPUT_H

class GLFWwindow;
class World;

class MouseInput {
private:
    World& world;
    Camera& camera;
    float lastX;
    float lastY;
    bool firstMouse;

public:

    MouseInput(Camera& _camera, World& _world);

    void processMouse(GLFWwindow* window, double xPosIn, double yPosIn);

    static void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn);

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};

#endif //CRAFTMINE_MOUSEINPUT_H
