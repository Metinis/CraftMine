#include "SceneRenderer.h"

#include "WorldGen/Chunk.h"
#include "Graphics/UBO.h"

float SceneRenderer::cameraNearPlane = 0.1f;
float SceneRenderer::cameraFarPlane = 500.0f;
std::vector<float> SceneRenderer::shadowCascadeLevels{ SceneRenderer::cameraFarPlane / 50.0f, SceneRenderer::cameraFarPlane / 25.0f, SceneRenderer::cameraFarPlane / 10.0f, SceneRenderer::cameraFarPlane / 2.0f };

SceneRenderer::SceneRenderer(Camera& _camera, Player& _player) : camera(_camera), player(_player){
    initialiseWorldShaders();
    initialiseShadowMap();
    initialiseGBuffer();
    ui = new Crosshair();
    playerBoxRenderer = new PlayerBoxRenderer();
    textRenderer = new TextRenderer(SOURCE_DIR "/resources/fonts/default.ttf", 18.0f);
}


int SceneRenderer::SHADOW_RESOLUTION = 1024 * 4;


void SceneRenderer::initialiseWorldShaders(){
    shader = new Shader(SOURCE_DIR "/resources/shader/shader.vs", SOURCE_DIR "/resources/shader/shader.fs");

    outlineShader = new Shader(SOURCE_DIR "/resources/shader/OutlineShader.vs", SOURCE_DIR "/resources/shader/OutlineShader.fs");

    transparentShader = new Shader(SOURCE_DIR "/resources/shader/transparentShader.vs", SOURCE_DIR "/resources/shader/transparentShader.fs");

    geometryShader = new Shader(SOURCE_DIR "/resources/shader/gShader.vs", SOURCE_DIR "/resources/shader/gShader.fs");

    worldTexture = new Texture(SOURCE_DIR "/resources/texture/terrain1.png");

    guiTexture = new Texture(SOURCE_DIR "/resources/gui/gui.png");

    inventoryTexture = new Texture(SOURCE_DIR "/resources/gui/inventory.png");

    cursorBlock = new CursorBlock();

    // Get the primary monitor
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    // Get the video mode of the primary monitor
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    // Calculate the aspect ratio
    const float aspectRatio = static_cast<float>(mode->width) / static_cast<float>(mode->height);


    model = glm::mat4(1.0f);
    view = glm::mat4(1.0f);
    proj = glm::perspective(glm::radians(65.0f), aspectRatio, cameraNearPlane, cameraFarPlane);

    outlineShader->use();
    outlineShader->setMat4("model", model);
    outlineShader->setMat4("projection", proj);

    geometryShader->use();
    geometryShader->setMat4("model", model);
    geometryShader->setMat4("projection", proj);
    //geometryShader->setMat4("view", view);

    shader->use();
    shader->setInt("gPosition", 0);
    shader->setInt("gNormal", 1);
    shader->setInt("gAlbedoSpec", 2);
    shader->setFloat("farPlane", cameraFarPlane);
    for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
    {
        shader->setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
    }

    shader->setInt("cascadeCount", static_cast<int>(shadowCascadeLevels.size()));


    //shader->setInt("depthMap", 3);
    glUniform1i(glGetUniformLocation(shader->ID, "depthMap"), 3);
    shader->setVec3("lightColor", glm::vec3(1.0f, 1.0f,1.0f));
    transparentShader->use();
    transparentShader->setFloat("farPlane", cameraFarPlane);
    for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
    {
        transparentShader->setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
    }

    transparentShader->setInt("cascadeCount", static_cast<int>(shadowCascadeLevels.size()));


    //transparentShader->setInt("depthMap", 3);
    glUniform1i(glGetUniformLocation(transparentShader->ID, "shadowMap"), 3);
    transparentShader->setInt("ourTexture", 0);
    transparentShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f,1.0f));
    transparentShader->setMat4("projection", proj);


    loadShader(*shader, World::viewDistance);
    loadShader(*transparentShader, World::viewDistance);

    ubo = new UBO();
}
void SceneRenderer::initialiseShadowMap(){

    screenQuad = new ScreenQuad();

    int x, y, width, height;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);
    fbo = new FBO(width, height);
    fbo->bindForRender();
    fbo->initialiseTextureFBO();

    frameShader = new Shader(SOURCE_DIR "/resources/shader/framebuffer.vs", SOURCE_DIR "/resources/shader/framebuffer.fs");

    frameShader->use();

    glUniform1i(glGetUniformLocation(frameShader->ID, "sampledTexture"), 0);

    shadowMapShader = new Shader(SOURCE_DIR "/resources/shader/shadowMap.vs", SOURCE_DIR "/resources/shader/shadowMap.fs", SOURCE_DIR "/resources/shader/shadowMap.gs");

    shadowMapShader->use();

    shadowMapShader->setMat4("model", model);

    depthFBO = new FBO(SHADOW_RESOLUTION, SHADOW_RESOLUTION);

    depthFBO->initialiseDepthFBO();

    updateShadowProjection();
}
void SceneRenderer::updateShadowProjection(){
    //lightPos = glm::vec3(glm::round(camera.position.x + sunXOffset), Chunk::HEIGHT + 100, glm::round(camera.position.z + sunZOffset));
    lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, -0.5f));

    if(std::abs(sunXOffset) > 400 && minBrightness > 0.3f){
        minBrightness -= 0.00001;
    }
    else if(std::abs(sunXOffset) < 400 && minBrightness < 0.5f){
        minBrightness += 0.00001;
    }
    if(std::abs(sunXOffset) > 400 && maxBrightnessFactor > 0.3f){
        maxBrightnessFactor -= 0.00004;
    }
    else if(std::abs(sunXOffset) < 400 && maxBrightnessFactor < 1.0f){
        maxBrightnessFactor += 0.00004;
    }


    const std::vector<glm::mat4> lightSpaceMats = getLightSpaceMatrices();
    ubo->bind();
    for (size_t i = 0; i < lightSpaceMats.size(); ++i) {
        glBufferSubData(GL_UNIFORM_BUFFER, static_cast<long>(i * sizeof(glm::mat4x4)), sizeof(glm::mat4x4), &lightSpaceMats[i]);
    }
    UBO::unbind();

    const glm::vec3 normalizedLightDir = glm::normalize(lightDir);
    view = camera.GetViewMatrix();
    shader->use();
    shader->setMat4("view", view);
    shader->setFloat("minBrightness", minBrightness);
    shader->setFloat("maxBrightnessFactor", maxBrightnessFactor);
    shader->setVec3("lightDir", normalizedLightDir);

    transparentShader->use();
    transparentShader->setMat4("view", view);
    transparentShader->setFloat("minBrightness", minBrightness);
    transparentShader->setFloat("maxBrightnessFactor", maxBrightnessFactor);
    transparentShader->setVec3("lightDir", normalizedLightDir);
}

void SceneRenderer::renderMesh(Mesh& mesh, Shader& _shader){
        mesh.render(_shader);
}

void SceneRenderer::loadShader(const Shader& _shader, const int viewDistance) const{
    _shader.use();
    _shader.setMat4("model", model);
    _shader.setMat4("projection", proj);
    _shader.setVec3("cameraPos", camera.position);
    _shader.setFloat("fogStart", static_cast<float>(((viewDistance) - (viewDistance/3)) < viewDistance - 1 ? (viewDistance - (viewDistance/3)) : 0) * Chunk::SIZE);
    _shader.setFloat("fogEnd", static_cast<float>(viewDistance-1) * Chunk::SIZE);
    _shader.setVec3("fogColor", fogColor);
}
void SceneRenderer::changeGlobalTexture()
{
    //Updates every second
    const int currentTime = static_cast<int>(glfwGetTime());
    if(currentTime != lastTime)
    {
        if(lastTexture < 5) {
            lastTexture++;
        }
        else {
            lastTexture = 1;
        }
        lastTime = currentTime;
        std::stringstream path;
        path << SOURCE_DIR "/resources/texture/terrain" << lastTexture << ".png";
        const std::string texturePath = path.str();
        worldTexture->setTexture(texturePath.c_str());
    }
}

void SceneRenderer::updateShaders(){
    const auto currentTime = static_cast<float>(glfwGetTime());
    shader->use();
    const glm::vec3 position = camera.position;
    shader->setVec3("cameraPos", position);
    if(player.isHeadInWater()){
        fogColor = glm::vec3(0.1f, 0.1f, 0.1f);
        shader->setFloat("fogStart",  5);
        shader->setFloat("fogEnd", 8);
        shader->setVec3("fogColor", fogColor);
        transparentShader->use();

        transparentShader->setFloat("fogStart",  5);
        transparentShader->setFloat("fogEnd", 8);
        transparentShader->setVec3("fogColor", fogColor);
    }
    else{
        fogColor = glm::vec3(0.55f, 0.75f, 1.0f);
        const int viewDistance = World::viewDistance;
        shader->setFloat("fogStart", static_cast<float>((viewDistance - (viewDistance/3)) < viewDistance - 1 ? (viewDistance - (viewDistance/3)) : 0) * Chunk::SIZE);
        shader->setFloat("fogEnd", static_cast<float>(viewDistance-1) * Chunk::SIZE);
        shader->setVec3("fogColor", fogColor);
        transparentShader->use();
        transparentShader->setFloat("fogStart", static_cast<float>((viewDistance - (viewDistance/3)) < viewDistance - 1 ? (viewDistance - (viewDistance/3)) : 0) * Chunk::SIZE);
        transparentShader->setFloat("fogEnd", static_cast<float>(viewDistance-1) * Chunk::SIZE);
        transparentShader->setVec3("fogColor", fogColor);
    }
    transparentShader->use();
    view = camera.GetViewMatrix();
    outlineShader->use();
    outlineShader->setMat4("view", view);
    transparentShader->use();
    transparentShader->setMat4("view", view);
    transparentShader->setFloat("time",  currentTime);
    transparentShader->setVec3("cameraPos", position);
    geometryShader->use();
    geometryShader->setMat4("view", view);
}
void SceneRenderer::renderBlockOutline(const World& world)
{
    glm::ivec3 result;
    Chunk* currentChunk;
    if(world.RaycastBlockPos(camera.position, camera.Front, result, currentChunk)){
        const auto globalPos = glm::ivec3(result.x + currentChunk->chunkPosition.x * Chunk::SIZE, result.y, result.z + currentChunk->chunkPosition.y * Chunk::SIZE);
        if(globalPos != lastOutlinePos)
        {
            updateOutlineBuffers(globalPos, currentChunk->GetBlockID(result));
            drawOutline();
        }
        else
        {
            drawOutline();
        }
        lastOutlinePos = globalPos;
    }
}
void SceneRenderer::updateOutlineBuffers(const glm::ivec3& globalPos, const unsigned char blockID){
    const std::vector<glm::vec3> vertices = Block::GetOutline(globalPos, blockID);
    std::vector<GLuint> indices;

    if(outlineVAO != nullptr)
    {
        outlineVAO->Delete();
        delete outlineVAO;
        outlineVAO = nullptr;
    }
    if(outlineVBO != nullptr)
    {
        outlineVBO->Delete();
        delete outlineVBO;
        outlineVBO = nullptr;
    }
    if(outlineIBO != nullptr)
    {
        outlineIBO->Delete();
        delete outlineIBO;
        outlineIBO = nullptr;
    }
    //Calculates the indices for the vertices, reusing a method for chunks
    int _indexCount = 0;
    for (int i = 0; i < 6; i++)
    {
        indices.push_back(3 + _indexCount);
        indices.push_back(0 + _indexCount);
        indices.push_back(0 + _indexCount);
        indices.push_back(1 + _indexCount);
        indices.push_back(1 + _indexCount);
        indices.push_back(2 + _indexCount);
        indices.push_back(2 + _indexCount);
        indices.push_back(3 + _indexCount);

        _indexCount += 4;	//uses element index, since each face only has 4 indices, we increment this everytime any block face is added
    }

    outlineVAO = new VAO();

    outlineVBO = new VBO(vertices);
    outlineVBO->Bind();

    outlineVAO->LinkToVAO(outlineShader->getAttribLocation("aPos"), 3, *outlineVBO);

    VBO::Unbind();
    VAO::Unbind();

    outlineIBO = new IBO(indices);
}

void SceneRenderer::drawOutline() const
{
    constexpr int indexCount = 48;
    outlineShader->use();
    outlineVAO->Bind();
    outlineIBO->Bind();
    glDrawElements(GL_LINES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
    VAO::Unbind();
    IBO::Unbind();
}

void SceneRenderer::render(Shader& _shader, const World& world){
    //calls render function to world which provides it with chunk meshes and uses scene.renderMesh
    world.renderChunksToShader(_shader);
}
void SceneRenderer::render(const World& world){
    world.renderChunksToNormalShaders();
}
void SceneRenderer::renderToShadowMap(const World& world) const{
    glEnable(GL_DEPTH_TEST);

    depthFBO->bindForRender();
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    world.renderChunksToShadow(*shadowMapShader);
    glEnable(GL_BLEND);
    glCullFace(GL_FRONT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void SceneRenderer::renderWorld(const World& world){

    setFBODimensions(Game::currentWidth,Game:: currentHeight);


    setGBufferDimensions(Game::currentWidth,Game:: currentHeight);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.55f, 0.75f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    worldTexture->Bind();
    geometryShader->use();
    world.renderSolidMeshes(*geometryShader);
    //world.renderChunksToShader(*geometryShader);
    //world.renderTransparentMeshes(*geometryShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    fbo->bindForRender();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);


    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

    glActiveTexture(GL_TEXTURE3);
    depthFBO->bindForReadDepth();
    screenQuad->renderQuad(*shader);
    FBO::UnbindDepth();
    FBO::Unbind();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->ID);
    glBlitFramebuffer(0, 0, static_cast<int>(fbo->width), static_cast<int>(fbo->height), 0, 0,
        static_cast<int>(fbo->width), static_cast<int>(fbo->height), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->ID);


    transparentShader->use();
    glActiveTexture(GL_TEXTURE0);
    worldTexture->Bind();
    glActiveTexture(GL_TEXTURE4);  // Transparency depth map
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthFBO->texture);
    glUniform1i(glGetUniformLocation(transparentShader->ID, "shadowMap"), 4);

    world.renderTransparentMeshes(*transparentShader);

    if (remotePlayersPtr != nullptr && playerBoxRenderer != nullptr) {
        playerBoxRenderer->render(*remotePlayersPtr, view, proj);
    }

    renderBlockOutline(world);
    //render toolbar to world frame buffer for post processing if inventory open
    glActiveTexture(GL_TEXTURE0);
    if(inventoryOpen){
        glDisable(GL_DEPTH_TEST);

        guiTexture->Bind();
        player.toolbar->renderToolbar();
        worldTexture->Bind();
        player.toolbar->renderItems();
        guiTexture->Bind();
        player.toolbar->renderSlot();
        worldTexture->Bind();

        glEnable(GL_DEPTH_TEST);
    }
    FBO::Unbind();
}
void SceneRenderer::renderGUI() const{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    //render cross hair/ui
    guiTexture->Bind();
    ui->renderCrosshair();
    if(!inventoryOpen) {
        player.toolbar->renderToolbar();
        worldTexture->Bind();
        player.toolbar->renderItems();
        guiTexture->Bind();
        player.toolbar->renderSlot();
        worldTexture->Bind();
    }

    if(inventoryOpen){
        inventoryTexture->Bind();
        player.inventory->renderInventory();
        worldTexture->Bind();
        player.inventory->renderItems();
        if(cursorBlock->currentBlock != 0){
            cursorBlock->renderBlockOnCursor();
        }
    }

    // Render chat overlay
    if (textRenderer != nullptr && chatPtr != nullptr) {
        textRenderer->setScreenDimensions(Game::currentWidth, Game::currentHeight);
        chatPtr->render(*textRenderer, Game::currentWidth, Game::currentHeight, glfwGetTime());
    }

    glEnable(GL_DEPTH_TEST);
}
void SceneRenderer::setFBODimensions(const int width, const int height) const{
    fbo->setDimensionTexture(width, height);
}
void SceneRenderer::renderQuad() const{


    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fbo->bindForRead();
    frameShader->use();
    //GLint textureLocation = glGetUniformLocation(frameShader->ID, "sampledTexture");
    //glUniform1i(textureLocation, 0);
    //before rendering check if player is in water and apply water effect to frame shader, same for inventory
    frameShader->setBool("inWater", player.isHeadInWater());
    frameShader->setBool("inInventory", inventoryOpen);

    //depthFBO->bindForReadDepth();
    //depthFBO->bindForRender();
    screenQuad->renderQuad(*frameShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
}

void SceneRenderer::changeSlotToolbar(const int slot) const {
    player.toolbar->changeSlot(slot);
}

void SceneRenderer::initialiseGBuffer() {

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    // Get the video mode of the primary monitor
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mode->width, mode->height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mode->width, mode->height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mode->width, mode->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    constexpr unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)

    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,  mode->width, mode->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneRenderer::setGBufferDimensions(const int width, const int height) const {
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    // position color buffer
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
std::vector<glm::vec4> SceneRenderer::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    const auto inv = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt =
                    inv * glm::vec4(
                        2.0f * static_cast<float>(x) - 1.0f,
                        2.0f * static_cast<float>(y) - 1.0f,
                        2.0f * static_cast<float>(z) - 1.0f,
                        1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}
glm::mat4 SceneRenderer::getLightSpaceMatrix(const float nearPlane, const float farPlane) const{

    //Get the primary monitor
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    // Get the video mode of the primary monitor
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    const float aspectRatio = static_cast<float>(mode->width) / static_cast<float>(mode->height);


    const auto proj = glm::perspective(glm::radians(65.0f), aspectRatio, nearPlane, farPlane);

    const auto corners = getFrustumCornersWorldSpace(proj, camera.GetViewMatrix());

    auto center = glm::vec3(0, 0, 0);
    for (const auto& v : corners)
    {
        center += glm::vec3(v);
    }
    center /= corners.size();

    //glm::vec3 lightDir = glm::normalize(glm::vec3(5.0f, 200, 20.0f));
    //glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, 1.0f, -0.5f));  // A sample directional vector for sunlight
    //glm::vec3 lightDir = center - lightPos;
    const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Tune this parameter according to the scene
    constexpr float zMult = 10.0f;
    if (minZ < 0)
    {
        minZ *= zMult;
    }
    else
    {
        minZ /= zMult;
    }
    if (maxZ < 0)
    {
        maxZ /= zMult;
    }
    else
    {
        maxZ *= zMult;
    }

    const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    return lightProjection * lightView;
}
std::vector<glm::mat4> SceneRenderer::getLightSpaceMatrices() const
{
    std::vector<glm::mat4> ret;
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(getLightSpaceMatrix(cameraNearPlane, shadowCascadeLevels[i]));
        }
        else if (i < shadowCascadeLevels.size())
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
        }
        else
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], cameraFarPlane));
        }
    }
    return ret;
}



