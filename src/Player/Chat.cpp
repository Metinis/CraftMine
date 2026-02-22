#include "Chat.h"
#include "Graphics/TextRenderer.h"
#include <glfw/include/GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

Chat::Chat()
    : inputActive(false)
    , hasPendingSend(false)
    , ignoreNextChar(false)
{
}

void Chat::addMessage(const std::string& sender, const std::string& text) {
    ChatMessage msg;
    msg.sender = sender;
    msg.text = text;
    msg.timestamp = glfwGetTime();
    messages.push_back(msg);
    if (messages.size() > MAX_MESSAGES) {
        messages.pop_front();
    }
}

void Chat::openInput() {
    inputActive = true;
    ignoreNextChar = true;
    inputBuffer.clear();
}

void Chat::closeInput() {
    inputActive = false;
    inputBuffer.clear();
}

void Chat::handleCharInput(unsigned int codepoint) {
    if (!inputActive) return;
    if (ignoreNextChar) {
        ignoreNextChar = false;
        return;
    }
    if (codepoint >= 32 && codepoint < 127 && inputBuffer.size() < MAX_INPUT_LENGTH) {
        inputBuffer += static_cast<char>(codepoint);
    }
}

void Chat::handleKeyInput(int key, int action) {
    if (!inputActive) return;
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ENTER) {
        if (!inputBuffer.empty()) {
            pendingSend = inputBuffer;
            hasPendingSend = true;
        }
        closeInput();
    } else if (key == GLFW_KEY_ESCAPE) {
        closeInput();
    } else if (key == GLFW_KEY_BACKSPACE) {
        if (!inputBuffer.empty()) {
            inputBuffer.pop_back();
        }
    }
}

bool Chat::consumeSendBuffer(std::string& outMessage) {
    if (!hasPendingSend) return false;
    outMessage = pendingSend;
    pendingSend.clear();
    hasPendingSend = false;
    return true;
}

bool Chat::isInputActive() const {
    return inputActive;
}

void Chat::render(TextRenderer& textRenderer, int screenWidth, int screenHeight,
                  double currentTime) {
    float lineHeight = 20.0f;
    float padding = 10.0f;
    float inputY = static_cast<float>(screenHeight) - padding - lineHeight;

    // Draw input line when active
    if (inputActive) {
        // Blinking cursor
        bool showCursor = fmod(currentTime, 1.0) < 0.5;
        std::string inputLine = "> " + inputBuffer + (showCursor ? "_" : "");

        // Drop shadow
        textRenderer.renderText(inputLine, padding + 1, inputY + 1,
                                glm::vec3(0.0f), 1.0f);
        textRenderer.renderText(inputLine, padding, inputY,
                                glm::vec3(1.0f), 1.0f);
        inputY -= lineHeight;
    }

    // Draw recent messages (bottom-up)
    int count = 0;
    for (int i = static_cast<int>(messages.size()) - 1; i >= 0 && count < VISIBLE_MESSAGES; i--, count++) {
        const ChatMessage& msg = messages[i];

        // Calculate fade alpha
        float alpha = 1.0f;
        if (!inputActive) {
            double age = currentTime - msg.timestamp;
            if (age > FADE_START + FADE_DURATION) continue;
            if (age > FADE_START) {
                alpha = 1.0f - static_cast<float>((age - FADE_START) / FADE_DURATION);
            }
        }

        std::string line = "<" + msg.sender + "> " + msg.text;
        float y = inputY - static_cast<float>(count) * lineHeight;

        // Drop shadow
        textRenderer.renderText(line, padding + 1, y + 1,
                                glm::vec3(0.0f), alpha);
        textRenderer.renderText(line, padding, y,
                                glm::vec3(1.0f), alpha);
    }
}
