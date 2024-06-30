//
// Created by vodkasas on 03/02/24.
//

#include "Input.h"

Input::Input(Camera& _camera, World& _world, Scene& _scene, Player& _player, Game& _game) : lastX(1280 / 2.0f), lastY(720 / 2.0f),
                                                                                            camera(_camera), world(_world), scene(_scene), player(_player), game(_game), firstMouse(true) {}

void Input::processMouse(GLFWwindow *window, double xposIn, double yposIn) {
    if(isCursorLocked){
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
    if(scene.inventoryOpen){
        scene.cursorBlock->setMousePosCoordinates(xposIn, yposIn);
    }
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
    if(button == GLFW_MOUSE_BUTTON_LEFT && !scene.inventoryOpen){
        if(action == GLFW_PRESS && isCursorLocked)
        {
            world.BreakBlocks(camera.position, camera.Front);
        }
        else{
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            isCursorLocked = true;
        }
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && scene.inventoryOpen){
        double cursorX;
        double cursorY;
        int width;
        int height;
        glfwGetWindowSize(window, &width, &height);
        glfwGetCursorPos(window, &cursorX, &cursorY);
        if((cursorY / height) < 0.77f){
            unsigned char blockID = scene.inventory->determineSlotBlockID(cursorX / width, cursorY / height);
            if(scene.cursorBlock->currentBlock == 0) {
                scene.cursorBlock->currentBlock = 0;
                scene.cursorBlock->loadBlockRendering(blockID);
                scene.cursorBlock->setScreenDimensions(width, height);
            }
            else{
                scene.cursorBlock->currentBlock = 0;
                scene.cursorBlock->loadBlockRendering(0);
                scene.cursorBlock->setScreenDimensions(width, height);
            }
        }
        else //if mouse under inventory
        {
            int toolbarIndex = scene.inventory->determineToolbarIndex(cursorX / width, cursorY / height);
            if(toolbarIndex != -1){
                unsigned char blockID = scene.toolbar->getID(toolbarIndex);
                scene.toolbar->setID(scene.cursorBlock->currentBlock, toolbarIndex);
                scene.toolbar->loadItemsRendering();
                player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));

                scene.inventory->loadItemsRendering();
                scene.cursorBlock->currentBlock = 0;
                scene.cursorBlock->loadBlockRendering(blockID);
                scene.cursorBlock->setScreenDimensions(width, height);
            }
            else{
                scene.inventory->loadItemsRendering();
                scene.cursorBlock->currentBlock = 0;
                scene.cursorBlock->loadBlockRendering(0);
                scene.cursorBlock->setScreenDimensions(width, height);
            }


        }
    }
    if(button == GLFW_MOUSE_BUTTON_RIGHT && !scene.inventoryOpen){
        if(action == GLFW_PRESS && isCursorLocked)
        {
            world.PlaceBlocks(camera.position, camera.Front);
        }
    }
}
void Input::scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    if(isCursorLocked) {
        if (yoffset > 0) {
            scene.toolbar->changeSlotNegative();
            scene.player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
        } else if (yoffset < 0) {
            scene.toolbar->changeSlotPositive();
            scene.player.setBlockID(scene.toolbar->getID(scene.toolbar->slot));
        }
    }
}
void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    input->processKey(key, action, window);

}
void Input::processKey(int key, int action, GLFWwindow* window) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        double currentTime = glfwGetTime();
        if(currentTime - lastPressTime <= timeFrame)
            player.updateFlying();

        lastPressTime = currentTime;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS && !wireFrame)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if(glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS && wireFrame){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS && !fullscreen) {
        // Get the primary monitor
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

        // Get the video mode of the primary monitor
        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

        glfwGetWindowPos(window, &windowedXPos, &windowedYPos);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);


        // Switch the window to fullscreen mode
        glfwSetWindowMonitor(window, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        glfwSwapInterval(0);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        fullscreen = true;
    }
    else if(glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS && fullscreen){
        // Switch back to windowed mode
        glfwSetWindowMonitor(window, nullptr, windowedXPos, windowedYPos, windowedWidth, windowedHeight, 0);

        // Restore V-Sync (optional)
        //glfwSwapInterval(1);

        fullscreen = false;
    }
    if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
        std::cout<<"Flying: "<<player.isFlying<<"\n";
        std::cout<<"Jumping: "<<player.isJumping<<"\n";
        std::cout<<"Grounded: "<<player.isGrounded<<"\n";
        std::cout<<"Velocity Y: "<<player.playerVelocity.y<<"\n";
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
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && isCursorLocked){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        isCursorLocked = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !isCursorLocked){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        isCursorLocked = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS){
        if(World::viewDistance >= 15){
            World::viewDistance = 3;
            world.UpdateViewDistance(player.chunkPosition);
            scene.loadShader(*scene.shader, World::viewDistance);
            scene.loadShader(*scene.transparentShader, World::viewDistance);
            scene.updateShadowResolution();
        }
        else {
            World::viewDistance += 3;
            world.UpdateViewDistance(player.chunkPosition);
            scene.loadShader(*scene.shader, World::viewDistance);
            scene.loadShader(*scene.transparentShader, World::viewDistance);
            scene.updateShadowResolution();
        }

    }
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && scene.inventoryOpen)){
        scene.inventoryOpen = !scene.inventoryOpen;
        if(scene.inventoryOpen){
            // Save the current cursor position
            glfwGetCursorPos(window, &previousCursorX, &previousCursorY);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            isCursorLocked = false;
        }
        else{
            // Get the window size
            int width, height;
            glfwGetWindowSize(window, &width, &height);

            // Calculate the center position
            double centerX = width / 2.0;
            double centerY = height / 2.0;

            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            glfwSetCursorPos(window, previousCursorX, previousCursorY);
            isCursorLocked = true;

            scene.cursorBlock->currentBlock=0;
        }
    }
}
void Input::processInput(GLFWwindow* window, bool* wireframe, bool* keyProccessed, bool* _isFullscreen, Player& player, World& world, float& deltaTime, Scene& scene)
{

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
}