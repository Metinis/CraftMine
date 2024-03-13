#pragma once
#include "Shader.h"
#include "Texture.h"
#include "Input/Camera.h"
#include "FBO.h"
#include "Mesh.h"

class Scene {
private:
    Shader* shader;
    Shader* transparentShader;
    Shader* outlineShader;
    Shader* frameShader;
    Shader* shadowMapShader;

    Texture* worldTexture;

    Camera* camera;
    FBO* fbo;
    FBO* depthFBO;

    std::vector<Mesh> solidMeshes;
    std::vector<Mesh> transparentMeshes;

public:
    Scene();

    void setSolidMeshes(const std::vector<Mesh>& meshes) {
        solidMeshes.clear();
        for(Mesh mesh : meshes){
            solidMeshes.push_back(mesh);
        }
    }

    void setTransparentMeshes(const std::vector<Mesh>& meshes) {
        transparentMeshes.clear();
        for(Mesh mesh : meshes){
            transparentMeshes.push_back(mesh);
        }
    }

    void renderSolidScene(Shader& _shader){
        for(Mesh mesh : solidMeshes){
            _shader.use();
            mesh.render();
        }
    }
    void renderTransparentScene(Shader& _shader){
        for(Mesh mesh : transparentMeshes){
            _shader.use();
            mesh.render();
        }
    }
};
