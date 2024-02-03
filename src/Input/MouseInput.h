//
// Created by vodkasas on 03/02/24.
//
#pragma once
#include "Camera.h"
#include"glfw-3.3.9/include/GLFW/glfw3.h"

#ifndef CRAFTMINE_MOUSEINPUT_H
#define CRAFTMINE_MOUSEINPUT_H

class GLFWwindow;

class MouseInput {
private:
    Camera& camera;
    float lastX;
    float lastY;
    bool firstMouse;

public:

    MouseInput(int SCR_WIDTH, int SCR_HEIGHT, Camera& _camera);

    void processMouse(GLFWwindow* window, double xPosIn, double yPosIn);

    static void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn);
};

#endif //CRAFTMINE_MOUSEINPUT_H
