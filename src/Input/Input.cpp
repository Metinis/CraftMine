//
// Created by vodkasas on 03/02/24.
//

#include "Input.h"

Input::Input(Camera& _camera, World& _world, Scene& _scene, Player& _player, Game& _game) : lastX(1280 / 2.0f), lastY(720 / 2.0f),
                                                                                            camera(_camera), world(_world), scene(_scene), player(_player), game(_game), firstMouse(true) {}

void Input::processMouse(GLFWwindow *window, double xposIn, double yposIn) {
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
void Input::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    Input* mouseInput = static_cast<Input*>(glfwGetWindowUserPointer(window));

    mouseInput->processMouse(window, xposIn, yposIn);
}
void Input::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    Input* mouseInput = static_cast<Input*>(glfwGetWindowUserPointer(window));

    mouseInput->mouseButtonCallback(window, button, action, mods);
}
void Input::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Input* mouseInput = static_cast<Input*>(glfwGetWindowUserPointer(window));

    mouseInput->scrollCallback(window, xoffset, yoffset);
}

void Input::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
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
void Input::scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    //TODO make this more clean and adjust player block with current toolbar items
    if (yoffset > 0) {
        scene.toolbar->changeSlotNegative();
        scene.player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
    } else if (yoffset < 0) {
        scene.toolbar->changeSlotPositive();
        scene.player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
    }
}
void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    input->processKey(key, action);

}
void Input::processKey(int key, int action) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        double currentTime = glfwGetTime();
        if(currentTime - lastPressTime <= timeFrame)
            player.updateFlying();

        lastPressTime = currentTime;
    }

}
void Input::processInput(GLFWwindow* window, bool* wireframe, bool* keyProccessed, bool* _isFullscreen, Player& player, World& world, float& deltaTime, Scene& scene)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS && !*wireframe && !*keyProccessed)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        *wireframe = true;
        *keyProccessed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS && *wireframe && !*keyProccessed)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        *wireframe = false;
        *keyProccessed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_RELEASE)
    {
        *keyProccessed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
        // Get the primary monitor
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

        // Get the video mode of the primary monitor
        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

        // Switch the window to fullscreen mode
        glfwSetWindowMonitor(window, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        glfwSwapInterval(0);
        *keyProccessed = true;
        *_isFullscreen = true;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
    }
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_RELEASE){
        *keyProccessed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
        std::cout<<"Flying: "<<player.isFlying<<"\n";
        std::cout<<"Jumping: "<<player.isJumping<<"\n";
        std::cout<<"Grounded: "<<player.isGrounded<<"\n";
        std::cout<<"Velocity Y: "<<player.playerVelocity.y<<"\n";
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        player.ProcessKeyboardMovement(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        player.ProcessKeyboardMovement(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        player.ProcessKeyboardMovement(cameraMovement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        player.ProcessKeyboardMovement(cameraMovement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        player.ProcessKeyboardMovement(cameraMovement::DOWN, deltaTime);
    else if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE){
        player.isShifting = false;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        player.ProcessKeyboardMovement(cameraMovement::UP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS){
        scene.changeSlotToolbar(0);
        player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS){
        scene.changeSlotToolbar(1);
        player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS){
        scene.changeSlotToolbar(2);
        player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS){
        scene.changeSlotToolbar(3);
        player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS){
        scene.changeSlotToolbar(4);
        player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS){
        scene.changeSlotToolbar(5);
        player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
    }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS){
        scene.changeSlotToolbar(6);
        player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
    }
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS){
        scene.changeSlotToolbar(7);
        player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
    }
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS){
        scene.changeSlotToolbar(8);
        player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }





}