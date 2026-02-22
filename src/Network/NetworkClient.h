#ifndef CRAFTMINE_NETWORKCLIENT_H
#define CRAFTMINE_NETWORKCLIENT_H
#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "PacketTypes.h"

class NetworkClient {
public:
    NetworkClient();
    ~NetworkClient();

    bool connect(const std::string& host, uint16_t port);
    void disconnect();
    bool isConnected() const;

    void sendPacket(uint8_t type, const std::vector<uint8_t>& payload);
    std::vector<Packet> processIncoming();

private:
    void recvThreadFunc();
    void sendThreadFunc();

    bool readExact(uint8_t* buffer, size_t length);

    int sockFd;
    std::atomic<bool> connected;
    std::atomic<bool> shutdownRequested;

    std::thread recvThread;
    std::thread sendThread;

    std::queue<Packet> incomingQueue;
    std::mutex incomingMutex;

    std::queue<Packet> sendQueue;
    std::mutex sendMutex;
    std::condition_variable sendCv;
};

#endif //CRAFTMINE_NETWORKCLIENT_H
