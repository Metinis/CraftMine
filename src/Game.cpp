//
// Created by vodkasas on 03/02/24.
//

#include "Game.h"
#include "Network/PacketTypes.h"
#include "WorldGen/WorldThreading.h"


Game::Game(const std::string& username) {
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
    chat = new Chat();
    scene = new SceneRenderer(*camera, *player);
    scene->remotePlayersPtr = &remotePlayers;
    scene->chatPtr = chat;
    world = new World(*camera, *scene, *player);
    player->world = world;

    mouseInput = new Input(*camera, *world, *scene, *player, *this);
    mouseInput->chat = chat;


    glfwSetWindowUserPointer(window, mouseInput);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, Input::mouse_callback);
    glfwSetMouseButtonCallback(window, Input::mouse_button_callback);
    glfwSetScrollCallback(window, Input::scroll_callback);
    glfwSetKeyCallback(window, Input::key_callback);
    glfwSetCharCallback(window, Input::char_callback);

    deltaTime = 0.0f;
    lastFrame = 0.0f;

    lastChunkPos = glm::ivec2(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);
    newChunkPos = glm::ivec2(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);

    updateingInt = 1;

    network = new NetworkClient();
    multiplayerMode = false;
    if (network->connect("127.0.0.1", 25565)) {
        multiplayerMode = true;
        world->multiplayerMode = true;
        world->networkClient = network;

        std::vector<uint8_t> helloPayload = PacketSerializer::serializeHello(username, "");
        network->sendPacket(PacketType::C2S_HELLO, helloPayload);
        std::cout << "[Game] Multiplayer mode enabled, sent Hello" << std::endl;
    } else {
        std::cout << "[Game] No server found, running singleplayer" << std::endl;
    }

    world->UpdateViewDistance(newChunkPos);
}

void Game::run() {

    const std::chrono::milliseconds TICK_DURATION(1000 / TICKS_PER_SECOND);
    auto previous = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds lag(0);
    // Disable VSync
    glfwSwapInterval(1);

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

        if (multiplayerMode) {
            processNetworkPackets();

            // Send pending chat messages
            std::string chatMsg;
            if (chat->consumeSendBuffer(chatMsg) && network != nullptr && network->isConnected()) {
                std::vector<uint8_t> payload = PacketSerializer::serializeChatMessage(chatMsg);
                network->sendPacket(PacketType::C2S_CHAT_MESSAGE, payload);
            }
        }

        currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        accumulator += deltaTime;



        while (accumulator >= timeStep) {
            // In multiplayer, wait for the spawn chunk to load before applying physics
            if (multiplayerMode && !spawnChunkReady) {
                glm::ivec2 spawnChunk(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);
                Chunk* c = world->GetChunk(spawnChunk);
                if (c != nullptr && c->generatedBlockData) {
                    spawnChunkReady = true;
                } else {
                    accumulator -= timeStep;
                    continue;
                }
            }
            // Update player physics
            player->Update(timeStep);

            if (multiplayerMode && network != nullptr && network->isConnected()) {
                positionSendTimer += timeStep;
                if (positionSendTimer >= 0.05f) {
                    std::vector<uint8_t> posPayload = PacketSerializer::serializePlayerPosition(
                        player->position.x, player->position.y, player->position.z,
                        camera->Yaw, camera->Pitch
                    );
                    network->sendPacket(PacketType::C2S_PLAYER_POSITION, posPayload);
                    positionSendTimer = 0.0f;
                }
            }

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
        Input::processInput(window, &wireframe, &keyProcessed, &isFullscreen, *player, *world, deltaTime, *scene, chat);

        if (deltaTime >= tickSpeed) {
            // Update sun offset for shadows
            //scene->sunXOffset -= 1 * deltaTime / 2;
            //if (scene->sunXOffset < -800) {
            //    scene->sunXOffset = 800;
            //}
                        //std::cout<<"\n called";
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
    if (network != nullptr) {
        network->disconnect();
        delete network;
        network = nullptr;
    }

    delete chat;
    chat = nullptr;

    glfwTerminate();
}

void Game::processNetworkPackets() {
    if (network == nullptr || !network->isConnected()) {
        if (multiplayerMode) {
            std::cout << "[Game] Lost connection to server, falling back to singleplayer" << std::endl;
            multiplayerMode = false;
            world->multiplayerMode = false;
            world->networkClient = nullptr;
        }
        return;
    }

    std::vector<Packet> packets = network->processIncoming();
    for (size_t i = 0; i < packets.size(); i++) {
        const Packet& pkt = packets[i];
        switch (pkt.type) {
            case PacketType::S2C_HELLO_ACK: {
                HelloAckPayload ack;
                if (PacketSerializer::deserializeHelloAck(pkt.payload, ack)) {
                    localPlayerId = ack.playerId;
                    std::cout << "[Game] Received HelloAck: spawn=("
                              << ack.spawnX << ", " << ack.spawnY << ", " << ack.spawnZ
                              << ") health=" << ack.health << std::endl;
                    player->position = glm::vec3(ack.spawnX, ack.spawnY, ack.spawnZ);
                    player->lastPosition = player->position;
                    player->camera.position = player->position;
                    newChunkPos = glm::ivec2(player->position.x / Chunk::SIZE, player->position.z / Chunk::SIZE);
                    lastChunkPos = newChunkPos;
                    world->UpdateViewDistance(newChunkPos);
                }
                break;
            }
            case PacketType::S2C_REJECT: {
                RejectPayload reject;
                if (PacketSerializer::deserializeReject(pkt.payload, reject)) {
                    std::cerr << "[Game] Server rejected connection: " << reject.reason << std::endl;
                }
                multiplayerMode = false;
                world->multiplayerMode = false;
                world->networkClient = nullptr;
                network->disconnect();
                break;
            }
            case PacketType::S2C_CHUNK_DATA: {
                ChunkDataPayload chunkData;
                if (PacketSerializer::deserializeChunkData(pkt.payload, chunkData)) {
                    world->receiveServerChunk(chunkData.chunkX, chunkData.chunkZ, chunkData.compressedData);
                }
                break;
            }
            case PacketType::S2C_PLAYER_JOIN: {
                PlayerJoinPayload join;
                if (PacketSerializer::deserializePlayerJoin(pkt.payload, join)) {
                    std::cout << "[Game] Player joined: " << join.username << " (id=" << join.playerId << ")" << std::endl;
                    remotePlayers[join.playerId] = RemotePlayer(
                        join.playerId, join.username,
                        glm::vec3(join.x, join.y, join.z), join.yaw, join.pitch
                    );
                }
                break;
            }
            case PacketType::S2C_PLAYER_LEAVE: {
                PlayerLeavePayload leave;
                if (PacketSerializer::deserializePlayerLeave(pkt.payload, leave)) {
                    std::cout << "[Game] Player left: id=" << leave.playerId << std::endl;
                    remotePlayers.erase(leave.playerId);
                }
                break;
            }
            case PacketType::S2C_PLAYER_POSITION: {
                PlayerPositionPayload pos;
                if (PacketSerializer::deserializePlayerPosition(pkt.payload, pos)) {
                    auto it = remotePlayers.find(pos.playerId);
                    if (it != remotePlayers.end()) {
                        it->second.lastPosition = it->second.position;
                        it->second.position = glm::vec3(pos.x, pos.y, pos.z);
                        it->second.yaw = pos.yaw;
                        it->second.pitch = pos.pitch;
                        it->second.interpTimer = 0.0f;
                    }
                }
                break;
            }
            case PacketType::S2C_BLOCK_CHANGE: {
                BlockChangePayload bc;
                if (PacketSerializer::deserializeBlockChange(pkt.payload, bc)) {
                    int chunkX = bc.x / Chunk::SIZE;
                    int chunkZ = bc.z / Chunk::SIZE;
                    int localX = bc.x % Chunk::SIZE;
                    int localZ = bc.z % Chunk::SIZE;
                    Chunk* chunk = world->GetChunk(chunkX, chunkZ);
                    if (chunk != nullptr && chunk->generatedBlockData) {
                        glm::ivec3 localPos(localX, bc.y, localZ);
                        chunk->SetBlock(localPos, bc.blockId);
                        WorldThreading::updateLoadData(chunk);
                        if (localX == 0 || localX == Chunk::SIZE - 1) {
                            int neighborX = (localX == 0) ? chunkX - 1 : chunkX + 1;
                            Chunk* neighbor = world->GetChunk(neighborX, chunkZ);
                            if (neighbor != nullptr && neighbor->generatedBlockData) {
                                WorldThreading::updateLoadData(neighbor);
                            }
                        }
                        if (localZ == 0 || localZ == Chunk::SIZE - 1) {
                            int neighborZ = (localZ == 0) ? chunkZ - 1 : chunkZ + 1;
                            Chunk* neighbor = world->GetChunk(chunkX, neighborZ);
                            if (neighbor != nullptr && neighbor->generatedBlockData) {
                                WorldThreading::updateLoadData(neighbor);
                            }
                        }
                    }
                }
                break;
            }
            case PacketType::S2C_CHAT_MESSAGE: {
                ChatMessagePayload chatData;
                if (PacketSerializer::deserializeChatMessage(pkt.payload, chatData)) {
                    chat->addMessage(chatData.username, chatData.message);
                }
                break;
            }
            case PacketType::S2C_PONG: {
                break;
            }
            default:
                std::cerr << "[Game] Unknown packet type: 0x"
                          << std::hex << static_cast<int>(pkt.type) << std::dec << std::endl;
                break;
        }
    }
}

int Game::currentWidth = 0;
int Game::currentHeight = 0;

void Game::framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    currentWidth = width;
    currentHeight = height;
}
