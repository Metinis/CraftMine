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
#include "Player/Crosshair.h"
#include "Player/Toolbar.h"
#include "Player/Inventory.h"
#include "CursorBlock.h"

class Scene {
private:
    static int SHADOW_RESOLUTION;
    static int SHADOW_DISTANCE;

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
    glm::vec3 fogColor = glm::vec3(0.55f, 0.75f, 1.0f);

    glm::vec3 lightPos = glm::vec3(0,0,0);

    VAO* outlineVAO = nullptr;
    VBO* outlineVBO = nullptr;
    IBO* outlineIBO = nullptr;

    Camera& camera;

    FBO* fbo;
    ScreenQuad* screenQuad;
    Crosshair* ui;

    unsigned int gBuffer;
    GLuint gPosition, gNormal, gAlbedoSpec;
    unsigned int rboDepth;

public:

    FBO* depthFBO;
    Shader* shader;
    Shader* transparentShader;
    Shader* outlineShader;
    Shader* frameShader;
    Shader* shadowMapShader;
    Shader* geometryShader;
    Texture* worldTexture;
    Texture* guiTexture;
    Texture* inventoryTexture;
    CursorBlock* cursorBlock;

    Player& player;

    Toolbar* toolbar;
    Inventory* inventory;

    float sunXOffset = 100;
    float sunYOffset = 50;

    bool inventoryOpen = false;
    unsigned char blockGrabbed = 0;

    Scene(Camera& _camera, Player& _player);

    void initialiseWorldShaders();

    void initialiseShadowMap();

    void updateShadowProjection();

    void loadShader(Shader& shader, const int viewDistance);

    void changeGlobalTexture();

    void updateShaders();

    void updateShadowResolution();

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

    void drawBlockOnCursor();

    void changeSlotToolbar(int slot);

    void initialiseGBuffer();

    void setGBufferDimensions(int width, int height);

    void renderGUI();

    void updateOutlineBuffers(glm::ivec3 &globalPos, unsigned char blockID);
};
