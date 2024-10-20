#include "Scene.h"

#include "Chunk.h"
#include "UBO.h"

float Scene::cameraNearPlane = 0.1f;
float Scene::cameraFarPlane = 500.0f;
std::vector<float> Scene::shadowCascadeLevels{ Scene::cameraFarPlane / 50.0f, Scene::cameraFarPlane / 25.0f, Scene::cameraFarPlane / 10.0f, Scene::cameraFarPlane / 2.0f };

Scene::Scene(Camera& _camera, Player& _player) : camera(_camera), player(_player){
    initialiseWorldShaders();
    initialiseShadowMap();
    initialiseGBuffer();
    ui = new Crosshair();
}


int Scene::SHADOW_RESOLUTION = 1024 * 4;


void Scene::initialiseWorldShaders(){
    shader = new Shader("../resources/shader/shader.vs", "../resources/shader/shader.fs");

    outlineShader = new Shader("../resources/shader/OutlineShader.vs", "../resources/shader/OutlineShader.fs");

    transparentShader = new Shader("../resources/shader/transparentShader.vs", "../resources/shader/transparentShader.fs");

    geometryShader = new Shader("../resources/shader/gShader.vs", "../resources/shader/gShader.fs");

    worldTexture = new Texture("../resources/texture/terrain1.png");

    guiTexture = new Texture("../resources/gui/gui.png");

    inventoryTexture = new Texture("../resources/gui/inventory.png");

    cursorBlock = new CursorBlock();

    // Get the primary monitor
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    // Get the video mode of the primary monitor
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    // Calculate the aspect ratio
    float aspectRatio = (float)mode->width / (float)mode->height;


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

    shader->setInt("cascadeCount", shadowCascadeLevels.size());


    shader->setInt("depthMap", 3);
    shader->setVec3("lightColor", glm::vec3(1.0f, 1.0f,1.0f));
    transparentShader->use();
     transparentShader->setFloat("farPlane", cameraFarPlane);
    for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
    {
        transparentShader->setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
    }

    transparentShader->setInt("cascadeCount", shadowCascadeLevels.size());


    transparentShader->setInt("depthMap", 3);
    transparentShader->setInt("ourTexture", 0);
    transparentShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f,1.0f));


    loadShader(*shader, World::viewDistance);
    loadShader(*transparentShader, World::viewDistance);

    ubo = new UBO();
}
void Scene::initialiseShadowMap(){

    screenQuad = new ScreenQuad();

    int x, y, width, height;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);
    fbo = new FBO(width, height);
    fbo->bindForRender();
    fbo->initialiseTextureFBO();

    frameShader = new Shader("../resources/shader/framebuffer.vs", "../resources/shader/framebuffer.fs");

    frameShader->use();

    glUniform1i(glGetUniformLocation(frameShader->ID, "sampledTexture"), 0);

    shadowMapShader = new Shader("../resources/shader/shadowMap.vs", "../resources/shader/shadowMap.fs", "../resources/shader/shadowMap.gs");

    shadowMapShader->use();

    shadowMapShader->setMat4("model", model);

    depthFBO = new FBO(SHADOW_RESOLUTION, SHADOW_RESOLUTION);

    depthFBO->initialiseDepthFBO();

    updateShadowProjection();
}
void Scene::updateShadowProjection(){
    //lightPos = glm::vec3(glm::round(camera.position.x + sunXOffset), Chunk::HEIGHT + 100, glm::round(camera.position.z + sunZOffset));

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


    std::vector<glm::mat4> lightSpaceMats = getLightSpaceMatrices();
    ubo->bind();
    for (size_t i = 0; i < lightSpaceMats.size(); ++i) {
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightSpaceMats[i]);
    }
    ubo->unbind();


    shader->use();
    view = camera.GetViewMatrix();
    shader->setMat4("view", view);

    //shader->setVec3("lightPos", lightPos);
    shader->setFloat("minBrightness", minBrightness);
    shader->setFloat("maxBrightnessFactor", maxBrightnessFactor);
    glm::vec3 lightDir = glm::normalize(glm::vec3(10.0f, 200, 20.0f));
    shader->setVec3("lightDir", lightDir);

    transparentShader->use();
    transparentShader->setMat4("view", view);

    //shader->setVec3("lightPos", lightPos);
    transparentShader->setFloat("minBrightness", minBrightness);
    transparentShader->setFloat("maxBrightnessFactor", maxBrightnessFactor);
    transparentShader->setVec3("lightDir", lightDir);

    //transparentShader->use();
    //transparentShader->setMat4("lightSpaceMatrix", lightProjection);
    //transparentShader->setVec3("lightPos", lightPos);
    //transparentShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f,1.0f));
    //transparentShader->setFloat("minBrightness", minBrightness);
    //transparentShader->setFloat("maxBrightnessFactor", maxBrightnessFactor);
}

void Scene::renderMesh(Mesh& mesh, Shader& _shader){
        mesh.render(_shader);
}

void Scene::loadShader(Shader& _shader, int viewDistance) {
    _shader.use();
    _shader.setMat4("model", model);
    _shader.setMat4("projection", proj);
    _shader.setVec3("cameraPos", camera.position);
    _shader.setFloat("fogStart", ((viewDistance - (viewDistance/3)) < viewDistance - 1 ? (viewDistance - (viewDistance/3)) : 0) * Chunk::SIZE);
    _shader.setFloat("fogEnd", (viewDistance-1) * Chunk::SIZE);
    _shader.setVec3("fogColor", fogColor);
}
void Scene::changeGlobalTexture()
{
    //Updates every second
    int currentTime = (int)glfwGetTime();
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
        path << "../resources/texture/terrain" << lastTexture << ".png";
        std::string texturePath = path.str();
        worldTexture->setTexture(texturePath.c_str());
    }
}

void Scene::updateShaders(){
    float currentTime = (float)glfwGetTime();
    shader->use();
    glm::vec3 position = camera.position;
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
        int viewDistance = World::viewDistance;
        shader->setFloat("fogStart", ((viewDistance - (viewDistance/3)) < viewDistance - 1 ? (viewDistance - (viewDistance/3)) : 0) * Chunk::SIZE);
        shader->setFloat("fogEnd", (viewDistance-1) * Chunk::SIZE);
        shader->setVec3("fogColor", fogColor);
        transparentShader->use();
        transparentShader->setFloat("fogStart", ((viewDistance - (viewDistance/3)) < viewDistance - 1 ? (viewDistance - (viewDistance/3)) : 0) * Chunk::SIZE);
        transparentShader->setFloat("fogEnd", (viewDistance-1) * Chunk::SIZE);
        transparentShader->setVec3("fogColor", fogColor);
    }
    transparentShader->use();
    transparentShader->setVec3("cameraPos", position);
    view = camera.GetViewMatrix();
    outlineShader->use();
    outlineShader->setMat4("view", view);
    transparentShader->use();
    transparentShader->setMat4("view", view);
    transparentShader->setFloat("time",  currentTime);
    geometryShader->use();
    geometryShader->setMat4("view", view);
}
void Scene::renderBlockOutline(World& world)
{
    glm::ivec3 result;
    Chunk* currentChunk;
    if(world.RaycastBlockPos(camera.position, camera.Front, result, currentChunk)){
        glm::ivec3 globalPos = glm::vec3(result.x + currentChunk->chunkPosition.x * Chunk::SIZE, result.y, result.z + currentChunk->chunkPosition.y * Chunk::SIZE);
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
void Scene::updateOutlineBuffers(glm::ivec3& globalPos, unsigned char blockID){
    std::vector<glm::vec3> vertices = Block::GetOutline(globalPos, blockID);
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

    outlineVBO->Unbind();
    outlineVAO->Unbind();

    outlineIBO = new IBO(indices);
}

void Scene::drawOutline()
{
    int indexCount = 48;
    outlineShader->use();
    outlineVAO->Bind();
    outlineIBO->Bind();
    glDrawElements(GL_LINES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
    outlineVAO->Unbind();
    outlineIBO->Unbind();
}

void Scene::render(Shader& _shader, World& world){
    //calls render function to world which provides it with chunk meshes and uses scene.renderMesh
    world.renderChunks(_shader);
}
void Scene::render(World& world){
    world.renderChunks();
}
void Scene::renderToShadowMap(World& world){
    glEnable(GL_DEPTH_TEST);

    depthFBO->bindForRender();
    glClear(GL_DEPTH_BUFFER_BIT);
    shadowMapShader->use();
    //GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    //if (status != GL_FRAMEBUFFER_COMPLETE)
    //{
    //    std::cout<<"Framebuffer incomplete";
    //}

    //glDisable(GL_BLEND);
    //glActiveTexture(GL_TEXTURE0);
    //worldTexture->Bind();
    //glCullFace(GL_BACK);
    world.renderChunks(*shadowMapShader);
    //glCullFace(GL_FRONT);
    //glEnable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Scene::renderWorld(World& world){

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
    glBlitFramebuffer(0, 0, fbo->width, fbo->height, 0, 0, fbo->width, fbo->height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->ID);


    transparentShader->use();
    glActiveTexture(GL_TEXTURE0);
    worldTexture->Bind();

    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);


    world.renderTransparentMeshes(*transparentShader);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    renderBlockOutline(world);
    //render toolbar to world frame buffer for post processing if inventory open
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
void Scene::renderGUI(){
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


    glEnable(GL_DEPTH_TEST);
}
void Scene::setFBODimensions(int width, int height){
    fbo->setDimensionTexture(width, height);
}
void Scene::renderQuad(){


    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fbo->bindForRead();
    frameShader->use();
    GLint textureLocation = glGetUniformLocation(frameShader->ID, "sampledTexture");
    glUniform1i(textureLocation, 0);
    //before rendering check if player is in water and apply water effect to frame shader, same for inventory
    frameShader->setBool("inWater", player.isHeadInWater());
    frameShader->setBool("inInventory", inventoryOpen);

    //depthFBO->bindForReadDepth();
    //depthFBO->bindForRender();
    screenQuad->renderQuad(*frameShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::changeSlotToolbar(int slot) {
    player.toolbar->changeSlot(slot);
}

void Scene::initialiseGBuffer() {

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    // Get the video mode of the primary monitor
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mode->width, mode->height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mode->width, mode->height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mode->width, mode->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
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

void Scene::setGBufferDimensions(int width, int height) {
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    // position color buffer
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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

void Scene::updateShadowResolution() {
    //Scene::SHADOW_RESOLUTION = 1024 * 4;
    //depthFBO->setDimensionDepthMap(SHADOW_RESOLUTION, SHADOW_RESOLUTION);
}

void Scene::drawBlockOnCursor() {

}
std::vector<glm::vec4> Scene::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
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
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}
glm::mat4 Scene::getLightSpaceMatrix(const float nearPlane, const float farPlane) {

    //Get the primary monitor
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    // Get the video mode of the primary monitor
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    float aspectRatio = (float)mode->width / (float)mode->height;


    const auto proj = glm::perspective(glm::radians(65.0f), aspectRatio, nearPlane, farPlane);

    const auto corners = getFrustumCornersWorldSpace(proj, camera.GetViewMatrix());

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners)
    {
        center += glm::vec3(v);
    }
    center /= corners.size();

    glm::vec3 lightDir = glm::normalize(glm::vec3(5.0f, 200, 20.0f));
    //glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));  // A sample directional vector for sunlight
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
std::vector<glm::mat4> Scene::getLightSpaceMatrices()
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



