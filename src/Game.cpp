//
// Created by vodkasas on 03/02/24.
//

#include "Game.h"


Game::Game(){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    wireframe = false;
    keyProcessed = false;
    isFullscreen = false;

    window = glfwCreateWindow(1280, 720, "CraftMine", nullptr, nullptr);

    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
    }

    glViewport(0, 0, 1280, 720);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    //glEnable(GL_MULTISAMPLE);



    //TODO make each accessible through scene to be more tidy
    player = new Player();
    camera = &player->camera;
    scene = new Scene(*camera, *player);
    world = new World(*camera, *scene, *player);
    player->world = world;


    mouseInput = new MouseInput(*camera, *world, *scene);


    glfwSetWindowUserPointer(window, mouseInput);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, MouseInput::mouse_callback);
    glfwSetMouseButtonCallback(window, MouseInput::mouse_button_callback);
    glfwSetScrollCallback(window, MouseInput::scroll_callback);

    deltaTime = 0.0f;
    lastFrame = 0.0f;

    lastChunkPos = glm::ivec2(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);
    newChunkPos = glm::ivec2(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);

    updateingInt = 1;
    world->UpdateViewDistance(newChunkPos);
}
void Game::run(){
    //render loop

    while(!glfwWindowShouldClose(window)) {
        if(!window)
        {
            break;
        }
        currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        accumulator += deltaTime;

        processInput(window, &wireframe, &keyProcessed, &isFullscreen, *player, *world, deltaTime, *scene);
        newChunkPos = (glm::vec2(glm::round(player->position.x) / Chunk::SIZE, glm::round(player->position.z) / Chunk::SIZE));

        if (std::abs(newChunkPos.x - lastChunkPos.x) >= updateingInt ||
            std::abs(newChunkPos.y - lastChunkPos.y) >= updateingInt) {
            lastChunkPos = newChunkPos;
            std::cout << newChunkPos.x << "x " << newChunkPos.y << "z \n";
            world->UpdateViewDistance(newChunkPos);


        }
        if(deltaTime >= tickSpeed){

            scene->sunXOffset -= 1 * deltaTime / 2;

            if(scene->sunXOffset < -800)
            {
                scene->sunXOffset = 800;
            }

            scene->updateShadowProjection();
        }


        world->update();
        //render shadow map

        scene->renderToShadowMap(*world);

        //render normal world
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        scene->setFBODimensions(width, height);

        scene->renderWorld(*world);

        glViewport(0, 0, width, height);

        //finally output FBO to quad
        scene->renderQuad();

        while(accumulator > timeStep){
            player->Update(timeStep);
            accumulator -= timeStep;
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void Game::processInput(GLFWwindow* window, bool* wireframe, bool* keyProccessed, bool* _isFullscreen, Player& player, World& world, float& deltaTime, Scene& scene)
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
        //if(!_isFullscreen && !*keyProccessed){
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
        //}
        /*else if(!*keyProccessed)
        {
            // Get the primary monitor
            GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

            // Get the video mode of the primary monitor
            const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

            // Switch the window to windowed mode
            glfwSetWindowMonitor(window, NULL, 0, 0, mode->width, mode->height, mode->refreshRate);
            *keyProccessed = true;
            *_isFullscreen = false;
        }*/
    }
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_RELEASE){
        *keyProccessed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
        std::cout<<world.blocksToBeAddedList[world.blocksToBeAddedList.size() - 1].localPosition.x << "x " << world.blocksToBeAddedList[0].localPosition.y<< "y " << world.blocksToBeAddedList[0].localPosition.z<< "z ";
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
        //player.shiftChanged = false;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !*keyProccessed && !*wireframe){
        player.ProcessKeyboardMovement(cameraMovement::UP, deltaTime);
        *wireframe = true;
        *keyProccessed = true;
    }
    else if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !*keyProccessed && *wireframe) {
        *wireframe = false;
        *keyProccessed = true;

    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        *keyProccessed = false;
    }
    //TODO update this
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

