//
// Created by denis on 2024-03-06.
//

#include "FBO.h"

#include "SceneRenderer.h"

FBO::FBO(const int _width, const int _height){
    width = _width;
    height = _height;

    /*Generate the FBO*/
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
}
void FBO::initialiseTextureFBO() {
    glGenTextures(1, &texture);
    initialiseTexture();

    /*Generate the RBO for depth info (Might change to shader for shadow maps)*/
    glGenRenderbuffers(1, &RBO);
    initialiseRBO();

    /*Unbind the framebuffer and textureFBO, should rebind for later use if needed*/
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);


    /*Check for incomplete framebuffer errors*/
    const auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer error: " << fboStatus << std::endl;
}
void FBO::initialiseDepthFBO(){
    glGenTextures(1, &texture);
    initialiseDepthMap();

    /*Unbind the framebuffer and textureFBO, should rebind for later use if needed*/
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);


    /*Check for incomplete framebuffer errors*/
    const auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer error: " << fboStatus << std::endl;
}
void FBO::setDimensionTexture(int _width, int _height){
    width = _width;
    height = _height;
    initialiseTexture();
    initialiseRBO();
}
void FBO::setDimensionDepthMap(int _width, int _height){
    width = _width;
    height = _height;
    initialiseDepthMap();
}
void FBO::initialiseTexture() const{

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(width), static_cast<int>(height), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
}
void FBO::initialiseDepthMap() const{
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexImage3D(
    GL_TEXTURE_2D_ARRAY,
    0,
    GL_DEPTH_COMPONENT32F,
    static_cast<int>(width),
    static_cast<int>(height),
    static_cast<int>(SceneRenderer::shadowCascadeLevels.size()) + 1,
    0,
    GL_DEPTH_COMPONENT,
    GL_FLOAT,
    nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // Prevents darkness outside the frustrum
    constexpr float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, clampColor);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);

    // Needed since we don't touch the color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void FBO::initialiseRBO() const{
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<int>(width), static_cast<int>(height));
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void FBO::bindForRender() const {
    glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

}
void FBO::bindFBOTextureLayer(const int layer) const {

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0, layer);
}
void FBO::bindForRead() const {

    glBindTexture(GL_TEXTURE_2D, texture);
}

void FBO::bindForReadDepth() const {

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
}

void FBO::Unbind(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void FBO::UnbindDepth() {

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void FBO::Delete() const
{
    glDeleteBuffers(1, &ID);
    glDeleteBuffers(1, &texture);
}