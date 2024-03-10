//
// Created by denis on 2024-03-06.
//

#include "FBO.h"
FBO::FBO(int _width, int _height){
    width = _width;
    height = _height;

    /*Generate the FBO*/
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    glGenTextures(1, &textureFBO);
    initialiseTexture();

    /*Generate the RBO for depth info (Might change to shader for shadow maps)*/

    glGenRenderbuffers(1, &RBO);
    initialiseRBO();

    /*Unbind the framebuffer and textureFBO, should rebind for later use if needed*/
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);


    /*Check for incomplete framebuffer errors*/
    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer error: " << fboStatus << std::endl;
}
void FBO::setDimension(int _width, int _height){
    width = _width;
    height = _height;
    initialiseTexture();
    //initialiseDepthMap();
    initialiseRBO();
}
void FBO::initialiseTexture() const{

    glBindTexture(GL_TEXTURE_2D, textureFBO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureFBO, 0);
}
void FBO::initialiseDepthMap() const{

    glBindTexture(GL_TEXTURE_2D, textureFBO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void FBO::initialiseRBO() const{
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
}
void FBO::bindForRender() const {
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
}
void FBO::bindForRead() const {
    glBindTexture(GL_TEXTURE_2D, textureFBO);
}
void FBO::Unbind(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::Delete()
{
    glDeleteBuffers(1, &ID);
    glDeleteBuffers(1, &textureFBO);
}