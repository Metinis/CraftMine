#include "Scene.h"

Scene::Scene(){
    shader = new Shader("../resources/shader/shader.vs", "../resources/shader/shader.fs");

    outlineShader = new Shader("../resources/shader/OutlineShader.vs", "../resources/shader/OutlineShader.fs");

    transparentShader = new Shader("../resources/shader/shader.vs", "../resources/shader/shader.fs"); //change when transparent is different

    worldTexture = new Texture("../resources/texture/terrain1.png");
}
