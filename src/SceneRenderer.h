#pragma once
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Input/Camera.h"
#include "Graphics/FBO.h"
#include "Graphics/Mesh.h"
#include "WorldGen/Chunk.h"
#include "WorldGen/World.h"
#include "Game.h"
#include "Graphics/ScreenQuad.h"
#include "Player/Crosshair.h"
#include "Player/Toolbar.h"
#include "Player/Inventory.h"
#include "Graphics/CursorBlock.h"

class UBO;

class SceneRenderer {
public:
    glm::mat4 view{};
    glm::mat4 proj{};

    static float cameraNearPlane;
    static float cameraFarPlane;
    static std::vector<float> shadowCascadeLevels;

    FBO* depthFBO{};
    UBO* ubo{};
    Shader* shader{};
    Shader* transparentShader{};
    Shader* outlineShader{};
    Shader* frameShader{};
    Shader* shadowMapShader{};
    Shader* geometryShader{};
    Texture* worldTexture{};
    Texture* guiTexture{};
    Texture* inventoryTexture{};
    CursorBlock* cursorBlock{};

    Player& player;

    float sunXOffset = 100;
    float sunYOffset = 50;

    bool inventoryOpen = false;
    unsigned char blockGrabbed = 0;

    SceneRenderer(Camera& _camera, Player& _player);
    void initialiseWorldShaders();
    void initialiseShadowMap();
    void updateShadowProjection();
    void updateProjection();
    void loadShader(const Shader& _shader, int viewDistance) const;
    void changeGlobalTexture();
    void updateShaders();
    void renderBlockOutline(const World& world);
    void drawOutline() const;
    static void renderMesh(Mesh& mesh, Shader& _shader);
    static void render(Shader& _shader, const World& world);
    static void render(const World& world);
    void renderToShadowMap(const World& world) const;
    void renderWorld(const World& world);
    void setFBODimensions(int width, int height) const;
    void renderQuad() const;
    void changeSlotToolbar(int slot) const;
    void initialiseGBuffer();
    void setGBufferDimensions(int width, int height) const;
    void renderGUI() const;
    void updateOutlineBuffers(const glm::ivec3 &globalPos, unsigned char blockID);
    static std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
    glm::mat4 getLightSpaceMatrix(float nearPlane, float farPlane) const;
    std::vector<glm::mat4> getLightSpaceMatrices() const;
private:
    int shadowRes{1024 * 4};
    glm::mat4 model{};
    int lastTexture = 1;
    int lastTime = 0;
    float minBrightness = 0.5f; //minimum brightness of the scene
    float maxBrightnessFactor = 1.0f; //max brightness mutliplier of the scene
    bool day = true;
    int sunZOffset = 25;
    glm::ivec3 lastOutlinePos{};
    glm::vec3 fogColor = glm::vec3(0.55f, 0.75f, 1.0f);
    glm::vec3 lightPos = glm::vec3(0,0,0);
    glm::vec3 lightDir = glm::vec3(20.0f, 200, 20.0f);
    VAO* outlineVAO = nullptr;
    VBO* outlineVBO = nullptr;
    IBO* outlineIBO = nullptr;
    Camera& camera;
    FBO* fbo{};
    ScreenQuad* screenQuad{};
    Crosshair* ui;
    unsigned int gBuffer{};
    GLuint gPosition{}, gNormal{}, gAlbedoSpec{};
    unsigned int rboDepth{};




};
