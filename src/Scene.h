#pragma once
#include "Shader.h"
#include "Texture.h"
#include "Input/Camera.h"
#include "FBO.h"
#include "Mesh.h"
#include "Chunk.h"
#include "World.h"
#include "Game.h"
#include "ScreenQuad.h"
#include "UI.h"



class Scene {
private:
    static const int SHADOW_RESOLUTION = 16384;

    glm::mat4 model{};
    glm::mat4 view{};
    glm::mat4 proj{};

    int lastTexture = 1;
    int lastTime = 0;

    int sunXOffset = 100;
    int sunZOffset = -200;

    glm::ivec3 lastOutlinePos;

    VAO* outlineVAO = nullptr;
    VBO* outlineVBO = nullptr;
    IBO* outlineIBO = nullptr;

    Camera& camera;
    FBO* fbo;
    FBO* depthFBO;
    ScreenQuad* screenQuad;
    UI* ui;

public:
    Shader* shader;
    Shader* transparentShader;
    Shader* outlineShader;
    Shader* frameShader;
    Shader* shadowMapShader;
    Texture* worldTexture;

    Scene(Camera& _camera);

    void initialiseWorldShaders();

    void initialiseShadowMap();

    void updateShadowProjection();

    void loadShader(Shader& shader, int viewDistance);

    void changeGlobalTexture();

    void updateShaders();

    void renderBlockOutline(World& world);

    void updateOutlineBuffers(glm::ivec3& globalPos);

    void drawOutline();

    static void renderMesh(Mesh& mesh, Shader& _shader);

    void render(Shader& _shader, World& world);

    void render(World& world);

    void renderToShadowMap(World& world);

    void renderWorld(World& world);

    void setFBODimensions(int width, int height);

    void renderQuad();
};