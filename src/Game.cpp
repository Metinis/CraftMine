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

    //player->position = glm::vec3(World::SIZE*Chunk::SIZE / 2, Chunk::HEIGHT, World::SIZE*Chunk::SIZE / 2);

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

    //TODO move to a screen quad/renderer class
    fbo = new FBO(SCR_WIDTH, SCR_HEIGHT);

    frameShader = new Shader("../resources/shader/framebuffer.vs", "../resources/shader/framebuffer.fs");

    frameShader->use();

    float rectangleVertices[] =
            {
                    // Coords    // texCoords
                    1.0f, -1.0f,  1.0f, 0.0f,
                    -1.0f, -1.0f,  0.0f, 0.0f,
                    -1.0f,  1.0f,  0.0f, 1.0f,

                    1.0f,  1.0f,  1.0f, 1.0f,
                    1.0f, -1.0f,  1.0f, 0.0f,
                    -1.0f,  1.0f,  0.0f, 1.0f
            };


    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));



    glUniform1i(glGetUniformLocation(frameShader->ID, "sampledTexture"), 0);

    float near_plane = 1.0f, far_plane = 7.5f;

    //frameShader->setFloat("near_plane", near_plane);
    //frameShader->setFloat("far_plane", far_plane);

    shadowMapShader = new Shader("../resources/shader/shadowMap.vs", "../resources/shader/shadowMap.fs");

    shadowMapShader->use();

    glGenFramebuffers(1, &shadowMapFBO);
    unsigned int shadowMapWidth = 1280, shadowMapHeight = 720;

    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // Prevents darkness outside the frustrum
    float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);

    // Needed since we don't touch the color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glm::mat4 orthgonalProjection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, 0.1f, 200.0f);
    glm::mat4 lightView = glm::lookAt(glm::vec3(player->position.x, 200.0f, player->position.z), glm::vec3(player->position.x, 0.0f, player->position.z), glm::vec3(0.0f,0.0f,-1.0f));
    glm::mat4 lightProjection = orthgonalProjection * lightView;

    glm::mat4 model = glm::mat4(1.0f);



    shadowMapShader->setMat4("model", model);

    glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));



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


            glm::mat4 orthgonalProjection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, 0.1f, 200.0f);
            glm::mat4 lightView = glm::lookAt(glm::vec3(player->position.x, 200.0f, player->position.z), glm::vec3(player->position.x, 0.0f, player->position.z - 100), glm::vec3(0.0f,0.0f,-1.0f));
            glm::mat4 lightProjection = orthgonalProjection * lightView;

            glm::mat4 model = glm::mat4(1.0f);

            shadowMapShader->setMat4("model", model);

            glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));
        }



        glEnable(GL_DEPTH_TEST);

        glViewport(0,0, 1280, 720);

        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
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
        glBindVertexArray(rectVAO);
        glDisable(GL_DEPTH_TEST);
        //fbo->bindForRead();

        glViewport(0, 0, width, height);
        frameShader->use();
        //shadowMapShader->use();
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        //glDisable(GL_CULL_FACE);
        glDrawArrays(GL_TRIANGLES, 0, 6);

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
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE){
        player.isShifting = false;
        //player.shiftChanged = false;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        player.ProcessKeyboardMovement(cameraMovement::UP, deltaTime);
}

