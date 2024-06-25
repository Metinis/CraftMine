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
        while(accumulator >= timeStep){
            player->Update(timeStep);
            newChunkPos = (glm::vec2(glm::round(player->position.x) / Chunk::SIZE, glm::round(player->position.z) / Chunk::SIZE));

            if (std::abs(newChunkPos.x - lastChunkPos.x) >= updateingInt ||
                std::abs(newChunkPos.y - lastChunkPos.y) >= updateingInt) {
                lastChunkPos = newChunkPos;
                std::cout << newChunkPos.x << "x " << newChunkPos.y << "z \n";
                world->UpdateViewDistance(newChunkPos);
            }
            accumulator -= timeStep;
        }
        const double alpha = accumulator / timeStep;

        //interpolate player pos and camera pos for smooth movement
        camera->updatePosition(player->lastPosition, player->position, alpha);

        Input::processInput(window, &wireframe, &keyProcessed, &isFullscreen, *player, *world, deltaTime, *scene);

        if(deltaTime >= tickSpeed){

            scene->sunXOffset -= 1 * deltaTime / 2;

            if(scene->sunXOffset < -800)
            {
                scene->sunXOffset = 800;
            }

            scene->updateShadowProjection();

            scene->renderToShadowMap(*world);
        }
        world->update();

        //render normal world
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        scene->setFBODimensions(width, height);
        scene->setGBufferDimensions(width, height);

        scene->renderWorld(*world);

        glViewport(0, 0, width, height);
        //finally output FBO to quad
        scene->renderQuad();



        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}





