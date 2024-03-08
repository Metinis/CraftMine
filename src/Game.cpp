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

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CraftMine", nullptr, nullptr);

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

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    player = new Player();
    camera = &player->camera;
    world = new World(*camera, *player);
    player->world = world;

    mouseInput = new MouseInput(SCR_WIDTH, SCR_HEIGHT, *camera, *world);
    ui = new UI();

    glfwSetWindowUserPointer(window, mouseInput);

    player->position = glm::vec3(World::SIZE*Chunk::SIZE / 2, Chunk::HEIGHT, World::SIZE*Chunk::SIZE / 2);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, MouseInput::mouse_callback);
    glfwSetMouseButtonCallback(window, MouseInput::mouse_button_callback);

    deltaTime = 0.0f;
    lastFrame = 0.0f;

    lastChunkPos = glm::ivec2(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);
    newChunkPos = glm::ivec2(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);

    updateingInt = 1; //world->viewDistance/2; //so it doesn't update every chunk
    world->UpdateViewDistance(newChunkPos);

    //glEnable(GL_DEPTH_TEST);

}
void Game::run(){
    //render loop

    while(!glfwWindowShouldClose(window)) {
        if(!window)
        {
            break;
        }
        auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, &wireframe, &keyProcessed, &isFullscreen, *player, *world, deltaTime);
        newChunkPos = (glm::vec2(glm::round(player->position.x) / Chunk::SIZE, glm::round(player->position.z) / Chunk::SIZE));

        if (std::abs(newChunkPos.x - lastChunkPos.x) >= updateingInt ||
            std::abs(newChunkPos.y - lastChunkPos.y) >= updateingInt) {
            lastChunkPos = newChunkPos;
            std::cout << newChunkPos.x << "x " << newChunkPos.y << "z \n";
            world->UpdateViewDistance(newChunkPos);
        }
        //glEnable(GL_DEPTH_TEST);
        glClearColor(0.55f, 0.75f, 1.0f, 1.0f);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        world->RenderWorld();
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA);
        ui->renderCrosshair();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        player->Update(deltaTime);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void Game::processInput(GLFWwindow* window, bool* wireframe, bool* keyProccessed, bool* _isFullscreen, Player& player, World& world, float& deltaTime)
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
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE){
        player.isShifting = false;
        //player.shiftChanged = false;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        player.ProcessKeyboardMovement(cameraMovement::UP, deltaTime);
}

