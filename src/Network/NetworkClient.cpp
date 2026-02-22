#include "NetworkClient.h"

#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

NetworkClient::NetworkClient()
    : sockFd(-1)
    , connected(false)
    , shutdownRequested(false)
{
}

NetworkClient::~NetworkClient() {
    disconnect();
}

bool NetworkClient::connect(const std::string& host, uint16_t port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[Network] WSAStartup failed" << std::endl;
        return false;
    }
#endif

    sockFd = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
    if (sockFd < 0) {
        std::cerr << "[Network] Failed to create socket" << std::endl;
        return false;
    }

    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "[Network] Invalid address: " << host << std::endl;
#ifdef _WIN32
        closesocket(sockFd);
#else
        close(sockFd);
#endif
        sockFd = -1;
        return false;
    }

    if (::connect(sockFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[Network] Connection failed to " << host << ":" << port << std::endl;
#ifdef _WIN32
        closesocket(sockFd);
#else
        close(sockFd);
#endif
        sockFd = -1;
        return false;
    }

    connected = true;
    shutdownRequested = false;

    recvThread = std::thread(&NetworkClient::recvThreadFunc, this);
    recvThread.detach();

    sendThread = std::thread(&NetworkClient::sendThreadFunc, this);
    sendThread.detach();

    std::cout << "[Network] Connected to " << host << ":" << port << std::endl;
    return true;
}

void NetworkClient::disconnect() {
    if (!connected && sockFd < 0) return;

    shutdownRequested = true;
    connected = false;

    sendCv.notify_all();

    if (sockFd >= 0) {
#ifdef _WIN32
        shutdown(sockFd, SD_BOTH);
        closesocket(sockFd);
#else
        shutdown(sockFd, SHUT_RDWR);
        close(sockFd);
#endif
        sockFd = -1;
    }
}

bool NetworkClient::isConnected() const {
    return connected;
}

void NetworkClient::sendPacket(uint8_t type, const std::vector<uint8_t>& payload) {
    if (!connected) return;

    Packet pkt;
    pkt.type = type;
    pkt.payload = payload;

    {
        std::lock_guard<std::mutex> lock(sendMutex);
        sendQueue.push(pkt);
    }
    sendCv.notify_one();
}

std::vector<Packet> NetworkClient::processIncoming() {
    std::vector<Packet> packets;
    std::lock_guard<std::mutex> lock(incomingMutex);
    while (!incomingQueue.empty()) {
        packets.push_back(incomingQueue.front());
        incomingQueue.pop();
    }
    return packets;
}

bool NetworkClient::readExact(uint8_t* buffer, size_t length) {
    size_t totalRead = 0;
    while (totalRead < length) {
        if (!connected || shutdownRequested) return false;

#ifdef _WIN32
        int n = recv(sockFd, reinterpret_cast<char*>(buffer + totalRead),
                     static_cast<int>(length - totalRead), 0);
#else
        ssize_t n = recv(sockFd, buffer + totalRead, length - totalRead, 0);
#endif
        if (n <= 0) {
            return false;
        }
        totalRead += static_cast<size_t>(n);
    }
    return true;
}

void NetworkClient::recvThreadFunc() {
    while (connected && !shutdownRequested) {
        // Read 2-byte length header (big-endian)
        uint8_t headerBuf[2];
        if (!readExact(headerBuf, 2)) {
            if (connected) {
                std::cerr << "[Network] Connection lost (read header)" << std::endl;
                connected = false;
            }
            return;
        }

        uint16_t packetLen = (static_cast<uint16_t>(headerBuf[0]) << 8) |
                              static_cast<uint16_t>(headerBuf[1]);

        if (packetLen < 1) {
            std::cerr << "[Network] Invalid packet length: " << packetLen << std::endl;
            connected = false;
            return;
        }

        // Read packet type + payload
        std::vector<uint8_t> data(packetLen);
        if (!readExact(data.data(), packetLen)) {
            if (connected) {
                std::cerr << "[Network] Connection lost (read payload)" << std::endl;
                connected = false;
            }
            return;
        }

        Packet pkt;
        pkt.type = data[0];
        pkt.payload.assign(data.begin() + 1, data.end());

        {
            std::lock_guard<std::mutex> lock(incomingMutex);
            incomingQueue.push(pkt);
        }
    }
}

void NetworkClient::sendThreadFunc() {
    while (connected && !shutdownRequested) {
        Packet pkt;
        {
            std::unique_lock<std::mutex> lock(sendMutex);
            sendCv.wait(lock, [this]() {
                return !sendQueue.empty() || shutdownRequested || !connected;
            });

            if (shutdownRequested || !connected) return;
            if (sendQueue.empty()) continue;

            pkt = sendQueue.front();
            sendQueue.pop();
        }

        // Frame: [u16 length (big-endian)][u8 type][payload...]
        uint16_t payloadLen = static_cast<uint16_t>(1 + pkt.payload.size());
        uint8_t header[2];
        header[0] = static_cast<uint8_t>((payloadLen >> 8) & 0xFF);
        header[1] = static_cast<uint8_t>(payloadLen & 0xFF);

        std::vector<uint8_t> frame;
        frame.reserve(2 + payloadLen);
        frame.push_back(header[0]);
        frame.push_back(header[1]);
        frame.push_back(pkt.type);
        frame.insert(frame.end(), pkt.payload.begin(), pkt.payload.end());

        size_t totalSent = 0;
        while (totalSent < frame.size()) {
            if (!connected || shutdownRequested) return;

#ifdef _WIN32
            int n = send(sockFd, reinterpret_cast<const char*>(frame.data() + totalSent),
                         static_cast<int>(frame.size() - totalSent), 0);
#else
            ssize_t n = send(sockFd, frame.data() + totalSent, frame.size() - totalSent, 0);
#endif
            if (n <= 0) {
                std::cerr << "[Network] Connection lost (send)" << std::endl;
                connected = false;
                return;
            }
            totalSent += static_cast<size_t>(n);
        }
    }
}
