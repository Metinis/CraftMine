#include "Scene.h"

Scene::Scene(Camera& _camera) : camera(_camera){
    initialiseWorldShaders();
    initialiseShadowMap();
    ui = new Crosshair();
    toolbar = new Toolbar();
}

const int Scene::SHADOW_RESOLUTION = 1024 * World::viewDistance;

void Scene::initialiseWorldShaders(){
    shader = new Shader("../resources/shader/shader.vs", "../resources/shader/shader.fs");

    outlineShader = new Shader("../resources/shader/OutlineShader.vs", "../resources/shader/OutlineShader.fs");

    transparentShader = new Shader("../resources/shader/shader.vs", "../resources/shader/shader.fs"); //change when transparent is different

    worldTexture = new Texture("../resources/texture/terrain1.png");

    guiTexture = new Texture("../resources/gui/gui.png");

    model = glm::mat4(1.0f);
    view = glm::mat4(1.0f);
    proj = glm::perspective(glm::radians(65.0f), 16.0f / 9.0f, 0.2f, 10000.0f);

    outlineShader->use();
    outlineShader->setMat4("model", model);
    outlineShader->setMat4("projection", proj);

    loadShader(*shader, World::viewDistance);
    loadShader(*transparentShader, World::viewDistance);
}
void Scene::initialiseShadowMap(){

    screenQuad = new ScreenQuad();

    fbo = new FBO(1280, 720);
    fbo->initialiseTextureFBO();

    frameShader = new Shader("../resources/shader/framebuffer.vs", "../resources/shader/framebuffer.fs");

    frameShader->use();

    glUniform1i(glGetUniformLocation(frameShader->ID, "sampledTexture"), 0);

    shadowMapShader = new Shader("../resources/shader/shadowMap.vs", "../resources/shader/shadowMap.fs");


    depthFBO = new FBO(SHADOW_RESOLUTION, SHADOW_RESOLUTION);
    depthFBO->initialiseDepthFBO();

    updateShadowProjection();
}
void Scene::updateShadowProjection(){
    shadowMapShader->use();

    glm::vec3 lightPos = glm::vec3(glm::round(camera.position->x + sunXOffset), 200, glm::round(camera.position->z + sunZOffset));

    if(std::abs(sunXOffset) > 400 && minBrightness > 0.3f){
        minBrightness -= 0.001;
    }
    else if(std::abs(sunXOffset) < 400 && minBrightness < 0.5f){
        minBrightness += 0.001;
    }
    if(std::abs(sunXOffset) > 400 && maxBrightnessFactor > 0.3f){
        maxBrightnessFactor -= 0.004;
    }
    else if(std::abs(sunXOffset) < 400 && maxBrightnessFactor < 1.0f){
        maxBrightnessFactor += 0.004;
    }
    float halfOrthoSize = World::viewDistance * Chunk::SIZE;
    float zFar = glm::round(float(halfOrthoSize + 200 + std::abs(sunZOffset)));

    glm::mat4 orthgonalProjection = glm::ortho(-halfOrthoSize, halfOrthoSize, -halfOrthoSize, halfOrthoSize, 0.1f, zFar);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(glm::round(camera.position->x), 50.0f, glm::round(camera.position->z)), glm::vec3(0.0f,0.0f,-1.0f));
    glm::mat4 lightProjection = orthgonalProjection * lightView;

    glm::mat4 model = glm::mat4(1.0f);

    shadowMapShader->setMat4("model", model);

    glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));

    glActiveTexture(GL_TEXTURE1);
    depthFBO->bindForRead();

    shader->use();
    shader->setMat4("lightSpaceMatrix", lightProjection);
    shader->setVec3("lightPos", lightPos);
    shader->setFloat("minBrightness", minBrightness);
    shader->setFloat("maxBrightnessFactor", maxBrightnessFactor);

    glUniform1i(glGetUniformLocation(shader->ID, "depthMap"), 1);

    transparentShader->use();
    transparentShader->setMat4("lightSpaceMatrix", lightProjection);
    transparentShader->setVec3("lightPos", lightPos);
    transparentShader->setFloat("minBrightness", minBrightness);
    shader->setFloat("maxBrightnessFactor", maxBrightnessFactor);

    glUniform1i(glGetUniformLocation(transparentShader->ID, "depthMap"), 1);
    glActiveTexture(GL_TEXTURE0);
}

void Scene::renderMesh(Mesh& mesh, Shader& _shader){
    _shader.use();
    mesh.render(_shader);
}

void Scene::loadShader(Shader& _shader, int viewDistance) {
    _shader.use();
    _shader.setMat4("model", model);
    _shader.setMat4("projection", proj);
    _shader.setVec3("cameraPos", *camera.position);
    _shader.setFloat("fogStart", ((viewDistance - (viewDistance/3)) < viewDistance - 1 ? (viewDistance - (viewDistance/3)) : 0) * Chunk::SIZE);
    _shader.setFloat("fogEnd", (viewDistance-1) * Chunk::SIZE);
    _shader.setVec3("fogColor", glm::vec3(0.55f, 0.75f, 1.0f));
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
    shader->use();
    //std::cout<< (int)((camera.position->x));
    if(camera.position == nullptr){
        std::cout<<"bug";
    }
    glm::vec3 position = *camera.position;
    shader->setVec3("cameraPos", position);
    transparentShader->use();
    transparentShader->setVec3("cameraPos", position);
    view = camera.GetViewMatrix();
    outlineShader->use();
    outlineShader->setMat4("view", view);
    shader->use();
    shader->setMat4("view", view);
    transparentShader->use();
    transparentShader->setMat4("view", view);
}
void Scene::renderBlockOutline(World& world)
{
    glm::ivec3 result;
    Chunk* currentChunk;
    if(world.RaycastBlockPos(*camera.position, camera.Front, result, currentChunk)){
        glm::ivec3 globalPos = glm::vec3(result.x + currentChunk->chunkPosition.x * Chunk::SIZE, result.y, result.z + currentChunk->chunkPosition.y * Chunk::SIZE);
        if(globalPos != lastOutlinePos)
        {
            updateOutlineBuffers(globalPos);
            drawOutline();
        }
        else
        {
            drawOutline();
        }
        lastOutlinePos = globalPos;
    }
}
void Scene::updateOutlineBuffers(glm::ivec3& globalPos){
    std::vector<glm::vec3> vertices = Block::GetOutline(globalPos);
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
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout<<"Framebuffer incomplete";
    }
    render(*shadowMapShader, world);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Scene::renderWorld(World& world){

    fbo->bindForRender();

    glClearColor(0.55f, 0.75f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    worldTexture->Bind();

    //world.update();
    render(world);
    renderBlockOutline(world);

    //render cross hair/ui
    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    ui->renderCrosshair();
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    guiTexture->Bind();
    //toolbar->changeSlot(0);
    toolbar->renderToolbar();

    fbo->Unbind();
    glDisable(GL_DEPTH_TEST);
}
void Scene::setFBODimensions(int width, int height){
    fbo->setDimension(width, height);
}
void Scene::renderQuad(){
    fbo->bindForRead();
    frameShader->use();
    screenQuad->renderQuad(*frameShader);
}

void Scene::changeSlotToolbar(int slot) {
    toolbar->changeSlot(slot);
}
