//
// Created by vodkasas on 03/02/24.
//

#include "MouseInput.h"

MouseInput::MouseInput(Camera& _camera, World& _world, Scene& _scene) : lastX(1280 / 2.0f), lastY(720/ 2.0f),
camera(_camera), world(_world), scene(_scene), firstMouse(true) {}

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
void MouseInput::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    MouseInput* mouseInput = static_cast<MouseInput*>(glfwGetWindowUserPointer(window));

    mouseInput->mouseButtonCallback(window, button, action, mods);
}
void MouseInput::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    MouseInput* mouseInput = static_cast<MouseInput*>(glfwGetWindowUserPointer(window));

    mouseInput->scrollCallback(window, xoffset, yoffset);
}

void MouseInput::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT){
        if(action == GLFW_PRESS)
        {
            world.BreakBlocks(*camera.position, camera.Front);
        }
    }
    if(button == GLFW_MOUSE_BUTTON_RIGHT){
        if(action == GLFW_PRESS)
        {
            world.PlaceBlocks(*camera.position, camera.Front);
        }
    }
}
void MouseInput::scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    //TODO make this more clean and adjust player block with current toolbar items
    if (yoffset > 0) {
        scene.toolbar->changeSlotNegative();
        scene.player.setBlockID(scene.toolbar->slot + 1);
    } else if (yoffset < 0) {
        scene.toolbar->changeSlotPositive();
        scene.player.setBlockID(scene.toolbar->slot + 1);
    }
}