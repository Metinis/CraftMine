//
// Created by vodkasas on 03/02/24.
//

#include "MouseInput.h"
MouseInput::MouseInput(int SCR_WIDTH, int SCR_HEIGHT, Camera& _camera) : lastX(SCR_WIDTH / 2.0f), lastY(SCR_HEIGHT / 2.0f), camera(_camera), firstMouse(true) {}

void MouseInput::processMouse(GLFWwindow *window, double xposIn, double yposIn) {
    auto xPos = static_cast<float>(xposIn);
    auto yPos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    float xOffset = xPos - lastX;
    float yOffset = lastY - yPos; // reversed since y-coordinates go from bottom to top

    lastX = xPos;
    lastY = yPos;

    // Assuming camera is an instance of some camera class
    camera.ProcessMouseMovement(xOffset, yOffset);
}
void MouseInput::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    MouseInput* mouseInput = static_cast<MouseInput*>(glfwGetWindowUserPointer(window));

    mouseInput->processMouse(window, xposIn, yposIn);
}