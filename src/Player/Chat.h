#ifndef CRAFTMINE_CHAT_H
#define CRAFTMINE_CHAT_H
#pragma once

#include <string>
#include <deque>

class TextRenderer;

struct ChatMessage {
    std::string sender;
    std::string text;
    double timestamp;
};

class Chat {
public:
    Chat();

    void addMessage(const std::string& sender, const std::string& text);
    void openInput();
    void closeInput();
    void handleCharInput(unsigned int codepoint);
    void handleKeyInput(int key, int action);
    bool consumeSendBuffer(std::string& outMessage);
    void render(TextRenderer& textRenderer, int screenWidth, int screenHeight, double currentTime);
    bool isInputActive() const;

private:
    static const size_t MAX_MESSAGES = 50;
    static const size_t MAX_INPUT_LENGTH = 200;
    static const int VISIBLE_MESSAGES = 10;
    static constexpr double FADE_START = 8.0;
    static constexpr double FADE_DURATION = 2.0;

    std::deque<ChatMessage> messages;
    std::string inputBuffer;
    bool inputActive;
    std::string pendingSend;
    bool hasPendingSend;
    bool ignoreNextChar;
};

#endif //CRAFTMINE_CHAT_H
