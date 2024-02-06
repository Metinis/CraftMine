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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    camera = new Camera();
    world = new World(*camera);
    mouseInput = new MouseInput(SCR_WIDTH, SCR_HEIGHT, *camera, *world);
    ui = new UI();

    glfwSetWindowUserPointer(window, mouseInput);

    camera->Position = glm::vec3(World::SIZE*Chunk::SIZE / 2, Chunk::HEIGHT, World::SIZE*Chunk::SIZE / 2);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, MouseInput::mouse_callback);
    glfwSetMouseButtonCallback(window, MouseInput::mouse_button_callback);

    deltaTime = 0.0f;
    lastFrame = 0.0f;

    lastChunkPos = glm::ivec2(camera->Position.x / Chunk::SIZE, camera->Position.z / Chunk::SIZE);
    newChunkPos = glm::ivec2(camera->Position.x / Chunk::SIZE, camera->Position.z / Chunk::SIZE);

    updateingInt = 1; //world->viewDistance/2; //so it doesn't update every chunk
    world->UpdateViewDistance(newChunkPos);
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

        processInput(window, &wireframe, &keyProcessed, *camera, *world, deltaTime);
        newChunkPos = glm::ivec2(camera->Position.x / Chunk::SIZE, camera->Position.z / Chunk::SIZE);

        if (std::abs(newChunkPos.x - lastChunkPos.x) >= updateingInt ||
            std::abs(newChunkPos.y - lastChunkPos.y) >= updateingInt) {
            lastChunkPos = newChunkPos;
            std::cout << newChunkPos.x << "x " << newChunkPos.y << "z \n";
            world->UpdateViewDistance(newChunkPos);
        }
        glClearColor(0.55f, 0.75f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ui->renderCrosshair();
        world->RenderWorld();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void Game::processInput(GLFWwindow* window, bool* wireframe, bool* keyProccessed, Camera& camera, World& world, float& deltaTime)
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
    }
    if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
        std::cout<<world.blocksToBeAddedList[world.blocksToBeAddedList.size() - 1].localPosition.x << "x " << world.blocksToBeAddedList[0].localPosition.y<< "y " << world.blocksToBeAddedList[0].localPosition.z<< "z ";
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(cameraMovement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(cameraMovement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(cameraMovement::DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(cameraMovement::UP, deltaTime);
}

