#define STB_TRUETYPE_IMPLEMENTATION
#include "TextRenderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <vector>

TextRenderer::TextRenderer(const char* fontPath, float fontSize)
    : atlasTexture(0), vao(0), vbo(0), shaderProgram(0), fontSize(fontSize)
{
    // Load font file
    std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "[TextRenderer] Failed to open font: " << fontPath << std::endl;
        return;
    }
    size_t fileSize = file.tellg();
    file.seekg(0);
    std::vector<unsigned char> fontBuffer(fileSize);
    file.read(reinterpret_cast<char*>(fontBuffer.data()), fileSize);
    file.close();

    // Bake font bitmap
    std::vector<unsigned char> atlasBitmap(ATLAS_WIDTH * ATLAS_HEIGHT);
    stbtt_BakeFontBitmap(fontBuffer.data(), 0, fontSize,
                         atlasBitmap.data(), ATLAS_WIDTH, ATLAS_HEIGHT,
                         FIRST_CHAR, NUM_CHARS, charData);

    // Upload atlas texture
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &atlasTexture);
    glBindTexture(GL_TEXTURE_2D, atlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ATLAS_WIDTH, ATLAS_HEIGHT, 0,
                 GL_RED, GL_UNSIGNED_BYTE, atlasBitmap.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create VAO/VBO with pre-allocated buffer
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Pre-allocate for up to 256 characters (6 vertices per char, 4 floats per vertex)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4 * 256, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Load shader
    shaderProgram = loadShader(SOURCE_DIR "/resources/shader/text.vs",
                               SOURCE_DIR "/resources/shader/text.fs");

    // Default projection
    projection = glm::ortho(0.0f, 1920.0f, 1080.0f, 0.0f);
}

TextRenderer::~TextRenderer() {
    if (atlasTexture) glDeleteTextures(1, &atlasTexture);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

void TextRenderer::setScreenDimensions(int width, int height) {
    projection = glm::ortho(0.0f, static_cast<float>(width),
                             static_cast<float>(height), 0.0f);
}

float TextRenderer::measureTextWidth(const std::string& text) const {
    float width = 0.0f;
    for (size_t i = 0; i < text.size(); i++) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (c < FIRST_CHAR || c >= FIRST_CHAR + NUM_CHARS) continue;
        const stbtt_bakedchar& bc = charData[c - FIRST_CHAR];
        width += bc.xadvance;
    }
    return width;
}

void TextRenderer::renderText(const std::string& text, float x, float y,
                               const glm::vec3& color, float alpha) {
    if (!shaderProgram || text.empty()) return;

    glUseProgram(shaderProgram);

    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    GLint colorLoc = glGetUniformLocation(shaderProgram, "textColor");
    glUniform3f(colorLoc, color.r, color.g, color.b);
    GLint alphaLoc = glGetUniformLocation(shaderProgram, "alpha");
    glUniform1f(alphaLoc, alpha);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlasTexture);
    GLint texLoc = glGetUniformLocation(shaderProgram, "fontAtlas");
    glUniform1i(texLoc, 0);

    std::vector<float> vertices;
    vertices.reserve(text.size() * 6 * 4);

    float curX = x;
    float curY = y;

    for (size_t i = 0; i < text.size(); i++) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (c < FIRST_CHAR || c >= FIRST_CHAR + NUM_CHARS) continue;

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(charData, ATLAS_WIDTH, ATLAS_HEIGHT,
                           c - FIRST_CHAR, &curX, &curY, &q, 1);

        // Two triangles per glyph
        float verts[] = {
            q.x0, q.y0, q.s0, q.t0,
            q.x1, q.y0, q.s1, q.t0,
            q.x1, q.y1, q.s1, q.t1,

            q.x0, q.y0, q.s0, q.t0,
            q.x1, q.y1, q.s1, q.t1,
            q.x0, q.y1, q.s0, q.t1,
        };
        vertices.insert(vertices.end(), verts, verts + 24);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 4));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GLuint TextRenderer::loadShader(const char* vertexPath, const char* fragmentPath) {
    // Read vertex shader
    std::ifstream vFile(vertexPath);
    if (!vFile) {
        std::cerr << "[TextRenderer] Failed to open vertex shader: " << vertexPath << std::endl;
        return 0;
    }
    std::string vCode((std::istreambuf_iterator<char>(vFile)),
                       std::istreambuf_iterator<char>());
    vFile.close();

    // Read fragment shader
    std::ifstream fFile(fragmentPath);
    if (!fFile) {
        std::cerr << "[TextRenderer] Failed to open fragment shader: " << fragmentPath << std::endl;
        return 0;
    }
    std::string fCode((std::istreambuf_iterator<char>(fFile)),
                       std::istreambuf_iterator<char>());
    fFile.close();

    const char* vSrc = vCode.c_str();
    const char* fSrc = fCode.c_str();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vSrc, nullptr);
    glCompileShader(vertex);
    GLint success;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(vertex, 512, nullptr, log);
        std::cerr << "[TextRenderer] Vertex shader error: " << log << std::endl;
    }

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fSrc, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(fragment, 512, nullptr, log);
        std::cerr << "[TextRenderer] Fragment shader error: " << log << std::endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "[TextRenderer] Shader link error: " << log << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}
