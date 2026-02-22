#ifndef CRAFTMINE_TEXTRENDERER_H
#define CRAFTMINE_TEXTRENDERER_H
#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

#include "stb_truetype.h"

class TextRenderer {
public:
    TextRenderer(const char* fontPath, float fontSize);
    ~TextRenderer();

    void renderText(const std::string& text, float x, float y,
                    const glm::vec3& color, float alpha);
    void setScreenDimensions(int width, int height);
    float measureTextWidth(const std::string& text) const;

private:
    static const int ATLAS_WIDTH = 512;
    static const int ATLAS_HEIGHT = 512;
    static const int FIRST_CHAR = 32;
    static const int NUM_CHARS = 96;

    stbtt_bakedchar charData[NUM_CHARS];
    GLuint atlasTexture;
    GLuint vao;
    GLuint vbo;
    GLuint shaderProgram;
    glm::mat4 projection;
    float fontSize;

    GLuint loadShader(const char* vertexPath, const char* fragmentPath);
};

#endif //CRAFTMINE_TEXTRENDERER_H
