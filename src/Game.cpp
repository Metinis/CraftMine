//
// Created by vodkasas on 03/02/24.
//

#include "Game.h"


Game::Game() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    wireframe = false;
    keyProcessed = false;
    isFullscreen = false;

    int x, y, width, height;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);


    window = glfwCreateWindow(width, height, "CraftMine", nullptr, nullptr);

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
    }

    glViewport(0, 0, width, height);

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
    World::viewDistance = 12;
    player = new Player();
    camera = &player->camera;
    scene = new Scene(*camera, *player);
    world = new World(*camera, *scene, *player);
    player->world = world;


    mouseInput = new Input(*camera, *world, *scene, *player, *this);


    glfwSetWindowUserPointer(window, mouseInput);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, Input::mouse_callback);
    glfwSetMouseButtonCallback(window, Input::mouse_button_callback);
    glfwSetScrollCallback(window, Input::scroll_callback);
    glfwSetKeyCallback(window, Input::key_callback);

    deltaTime = 0.0f;
    lastFrame = 0.0f;

    lastChunkPos = glm::ivec2(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);
    newChunkPos = glm::ivec2(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);

    updateingInt = 1;
    world->UpdateViewDistance(newChunkPos);
}

void Game::run() {

    const std::chrono::milliseconds TICK_DURATION(1000 / TICKS_PER_SECOND);
    auto previous = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds lag(0);
    // Disable VSync
    glfwSwapInterval(1);

    // Variables for FPS calculation
    int frames = 0;
    double fpsTime = 0.0;
    // Initial window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    currentWidth = width;
    currentHeight = height;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        if (!window) {
            break;
        }

        currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        accumulator += deltaTime;



        while (accumulator >= timeStep) {
            // Update player physics
            player->Update(timeStep);

            // Handle chunk position update
            newChunkPos = (glm::vec2(glm::round(player->position.x) / Chunk::SIZE,
                                     glm::round(player->position.z) / Chunk::SIZE));

            if (std::abs(newChunkPos.x - lastChunkPos.x) >= updateingInt ||
                std::abs(newChunkPos.y - lastChunkPos.y) >= updateingInt) {
                lastChunkPos = newChunkPos;
                std::cout << newChunkPos.x << "x " << newChunkPos.y << "z \n";
                world->UpdateViewDistance(newChunkPos);
            }
            accumulator -= timeStep;
        }

        // Interpolation factor for smooth rendering
        const double alpha = accumulator / timeStep;

        // Interpolate camera position based on physics update and alpha
        camera->updatePosition(player->lastPosition, player->position, alpha);

        // Process user input
        Input::processInput(window, &wireframe, &keyProcessed, &isFullscreen, *player, *world, deltaTime, *scene);

        if (deltaTime >= tickSpeed) {
            // Update sun offset for shadows
            scene->sunXOffset -= 1 * deltaTime / 2;
            if (scene->sunXOffset < -800) {
                scene->sunXOffset = 800;
            }
            scene->updateShadowProjection();
            scene->renderToShadowMap(*world);
        }
        auto current = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current - previous);
        previous = current;
        lag += elapsed;

        // Handle updates at the fixed tick rate (20 ticks per second)
        while (lag >= TICK_DURATION) {

            world->updateTick();
            lag -= TICK_DURATION;
        }

        // Update world
        world->update();

        // Check if window size has changed
        glfwGetFramebufferSize(window, &width, &height);

        if (width != currentWidth || height != currentHeight) {
            currentWidth = width;
            currentHeight = height;

            // Resize FBO and G-buffer to match the new window size
            scene->setFBODimensions(currentWidth, currentHeight);
            scene->setGBufferDimensions(currentWidth, currentHeight);


        }


        //glViewport(0, 0, width, height);
        // Render the world
        scene->renderWorld(*world);

        // Set viewport and render quad
        scene->renderQuad();

        // Render GUI
        scene->renderGUI();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();

        /* FPS calculation
        frames++;
        fpsTime += deltaTime;
        if (fpsTime >= 1.0) {
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
            fpsTime = 0.0;
        }*/
    }
    glfwTerminate();
}

int Game::currentWidth = 0;
int Game::currentHeight = 0;

void Game::framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    currentWidth = width;
    currentHeight = height;
}
