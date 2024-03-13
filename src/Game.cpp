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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, MouseInput::mouse_callback);
    glfwSetMouseButtonCallback(window, MouseInput::mouse_button_callback);

    deltaTime = 0.0f;
    lastFrame = 0.0f;

    lastChunkPos = glm::ivec2(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);
    newChunkPos = glm::ivec2(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);

    updateingInt = 1;
    world->UpdateViewDistance(newChunkPos);

    //glEnable(GL_DEPTH_TEST);

    screenQuad = new ScreenQuad();

    fbo = new FBO(SCR_WIDTH, SCR_HEIGHT);
    fbo->initialiseTextureFBO();

    frameShader = new Shader("../resources/shader/framebuffer.vs", "../resources/shader/framebuffer.fs");

    frameShader->use();

    glUniform1i(glGetUniformLocation(frameShader->ID, "sampledTexture"), 0);

    shadowMapShader = new Shader("../resources/shader/shadowMap.vs", "../resources/shader/shadowMap.fs");

    shadowMapShader->use();

    depthFBO = new FBO(16384, 16384);
    depthFBO->initialiseDepthFBO();

    glm::mat4 orthgonalProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 200.0f);
    glm::mat4 lightView = glm::lookAt(glm::vec3(player->position.x, 200.0f, player->position.z), glm::vec3(player->position.x, 0.0f, player->position.z), glm::vec3(0.0f,0.0f,-1.0f));
    glm::mat4 lightProjection = orthgonalProjection * lightView;

    glm::mat4 model = glm::mat4(1.0f);



    shadowMapShader->setMat4("model", model);

    glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));

    world->shader->use();
    world->shader->setMat4("lightSpaceMatrix", lightProjection);
    depthFBO->bindForRead();
    glUniform1i(glGetUniformLocation(world->shader->ID, "depthMap"), 0);


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

            shadowMapShader->use();


            glm::mat4 orthgonalProjection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, 0.1f, 400.0f);
            glm::mat4 lightView = glm::lookAt(glm::vec3(player->position.x, 200.0f, player->position.z + 200), glm::vec3(player->position.x, 50.0f, player->position.z), glm::vec3(0.0f,0.0f,-1.0f));
            glm::mat4 lightProjection = orthgonalProjection * lightView;

            glm::mat4 model = glm::mat4(1.0f);

            shadowMapShader->setMat4("model", model);

            glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));


            glActiveTexture(GL_TEXTURE1);
            depthFBO->bindForRead();

            world->shader->use();
            world->shader->setMat4("lightSpaceMatrix", lightProjection);

            glUniform1i(glGetUniformLocation(world->shader->ID, "depthMap"), 1);

            world->transparentShader->use();
            world->transparentShader->setMat4("lightSpaceMatrix", lightProjection);

            glUniform1i(glGetUniformLocation(world->transparentShader->ID, "depthMap"), 1);
            glActiveTexture(GL_TEXTURE0);
        }



        glEnable(GL_DEPTH_TEST);

        depthFBO->bindForRender();
        glClear(GL_DEPTH_BUFFER_BIT);
        shadowMapShader->use();
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
           std::cout<<"Framebuffer incomplete";
        }
        world->RenderShadowWorld(*shadowMapShader);
        //world->RenderWorld();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        fbo->setDimension(width, height);
        fbo->bindForRender();
        //glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

        glClearColor(0.55f, 0.75f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        world->texture->Bind();
        world->RenderWorld();
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA);
        ui->renderCrosshair();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        fbo->Unbind();
        glDisable(GL_DEPTH_TEST);
        fbo->bindForRead();

        glViewport(0, 0, width, height);
        frameShader->use();
        //glBindTexture(GL_TEXTURE_2D, shadowMap);
        screenQuad->renderQuad(*frameShader);

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



}

