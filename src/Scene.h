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
#include "Crosshair.h"
#include "Toolbar.h"

class Scene {
private:
    static const int SHADOW_RESOLUTION;

    glm::mat4 model{};
    glm::mat4 view{};
    glm::mat4 proj{};

    int lastTexture = 1;
    int lastTime = 0;
    float minBrightness = 0.5f; //minimum brightness of the scene
    float maxBrightnessFactor = 1.0f; //max brightness mutliplier of the scene
    bool day = true;


    int sunZOffset = 25;

    glm::ivec3 lastOutlinePos;

    VAO* outlineVAO = nullptr;
    VBO* outlineVBO = nullptr;
    IBO* outlineIBO = nullptr;

    Camera& camera;

    FBO* fbo;
    FBO* depthFBO;
    ScreenQuad* screenQuad;
    Crosshair* ui;


public:
    Shader* shader;
    Shader* transparentShader;
    Shader* outlineShader;
    Shader* frameShader;
    Shader* shadowMapShader;
    Texture* worldTexture;
    Texture* guiTexture;

    Player& player;

    Toolbar* toolbar;

    float sunXOffset = 100;
    float sunYOffset = 50;

    Scene(Camera& _camera, Player& _player);

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

    void changeSlotToolbar(int slot);
};
