#ifndef CRAFTMINE_PACKETTYPES_H
#define CRAFTMINE_PACKETTYPES_H
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace PacketType {
    static const uint8_t C2S_HELLO = 0x01;
    static const uint8_t S2C_HELLO_ACK = 0x02;
    static const uint8_t S2C_REJECT = 0x03;
    static const uint8_t S2C_PLAYER_JOIN = 0x10;
    static const uint8_t S2C_PLAYER_LEAVE = 0x11;
    static const uint8_t S2C_PLAYER_POSITION = 0x12;
    static const uint8_t C2S_PLAYER_POSITION = 0x13;
    static const uint8_t C2S_BLOCK_CHANGE = 0x20;
    static const uint8_t S2C_BLOCK_CHANGE = 0x21;
    static const uint8_t S2C_CHUNK_DATA = 0x30;
    static const uint8_t C2S_REQUEST_CHUNK = 0x31;
    static const uint8_t C2S_CHAT_MESSAGE = 0x40;
    static const uint8_t S2C_CHAT_MESSAGE = 0x41;
    static const uint8_t C2S_PING = 0x70;
    static const uint8_t S2C_PONG = 0x71;
}

static const size_t MAX_PACKET_SIZE = 200 * 1024;

struct Packet {
    uint8_t type;
    std::vector<uint8_t> payload;
};

struct HelloPayload {
    std::string username;
    std::string password;
};

struct HelloAckPayload {
    uint32_t playerId;
    float spawnX;
    float spawnY;
    float spawnZ;
    float health;
};

struct RejectPayload {
    std::string reason;
};

struct ChunkDataPayload {
    int32_t chunkX;
    int32_t chunkZ;
    std::vector<uint8_t> compressedData;
};

struct RequestChunkPayload {
    int32_t chunkX;
    int32_t chunkZ;
};

struct PlayerJoinPayload {
    uint32_t playerId;
    std::string username;
    float x, y, z, yaw, pitch;
};

struct PlayerLeavePayload {
    uint32_t playerId;
};

struct PlayerPositionPayload {
    uint32_t playerId;
    float x, y, z, yaw, pitch;
};

struct BlockChangePayload {
    int32_t x, y, z;
    uint8_t blockId;
};

struct ChatMessagePayload {
    uint32_t playerId;
    std::string username;
    std::string message;
};

namespace PacketSerializer {

inline float ntohf(uint32_t netVal) {
    uint32_t hostVal = ntohl(netVal);
    float result;
    std::memcpy(&result, &hostVal, sizeof(float));
    return result;
}

inline uint32_t htonf(float hostVal) {
    uint32_t raw;
    std::memcpy(&raw, &hostVal, sizeof(float));
    return htonl(raw);
}

inline std::vector<uint8_t> serializeHello(const std::string& username, const std::string& password) {
    std::vector<uint8_t> payload;
    payload.push_back(static_cast<uint8_t>(username.size()));
    payload.insert(payload.end(), username.begin(), username.end());
    payload.push_back(static_cast<uint8_t>(password.size()));
    payload.insert(payload.end(), password.begin(), password.end());
    return payload;
}

inline std::vector<uint8_t> serializeRequestChunk(int32_t chunkX, int32_t chunkZ) {
    std::vector<uint8_t> payload(8);
    uint32_t nx = htonl(static_cast<uint32_t>(chunkX));
    uint32_t nz = htonl(static_cast<uint32_t>(chunkZ));
    std::memcpy(payload.data(), &nx, 4);
    std::memcpy(payload.data() + 4, &nz, 4);
    return payload;
}

inline std::vector<uint8_t> serializePing() {
    return std::vector<uint8_t>();
}

inline bool deserializeHelloAck(const std::vector<uint8_t>& payload, HelloAckPayload& out) {
    if (payload.size() < 20) return false;
    uint32_t rawId;
    std::memcpy(&rawId, payload.data(), 4);
    out.playerId = ntohl(rawId);

    uint32_t rawX, rawY, rawZ, rawH;
    std::memcpy(&rawX, payload.data() + 4, 4);
    std::memcpy(&rawY, payload.data() + 8, 4);
    std::memcpy(&rawZ, payload.data() + 12, 4);
    std::memcpy(&rawH, payload.data() + 16, 4);
    out.spawnX = ntohf(rawX);
    out.spawnY = ntohf(rawY);
    out.spawnZ = ntohf(rawZ);
    out.health = ntohf(rawH);
    return true;
}

inline bool deserializeReject(const std::vector<uint8_t>& payload, RejectPayload& out) {
    if (payload.size() < 1) return false;
    uint8_t len = payload[0];
    if (payload.size() < 1u + len) return false;
    out.reason = std::string(payload.begin() + 1, payload.begin() + 1 + len);
    return true;
}

inline bool deserializeChunkData(const std::vector<uint8_t>& payload, ChunkDataPayload& out) {
    if (payload.size() < 8) return false;
    uint32_t rawX, rawZ;
    std::memcpy(&rawX, payload.data(), 4);
    std::memcpy(&rawZ, payload.data() + 4, 4);
    out.chunkX = static_cast<int32_t>(ntohl(rawX));
    out.chunkZ = static_cast<int32_t>(ntohl(rawZ));
    out.compressedData.assign(payload.begin() + 8, payload.end());
    return true;
}

inline std::vector<uint8_t> serializePlayerPosition(float x, float y, float z, float yaw, float pitch) {
    std::vector<uint8_t> payload(20);
    uint32_t nx = htonf(x);
    uint32_t ny = htonf(y);
    uint32_t nz = htonf(z);
    uint32_t nYaw = htonf(yaw);
    uint32_t nPitch = htonf(pitch);
    std::memcpy(payload.data(), &nx, 4);
    std::memcpy(payload.data() + 4, &ny, 4);
    std::memcpy(payload.data() + 8, &nz, 4);
    std::memcpy(payload.data() + 12, &nYaw, 4);
    std::memcpy(payload.data() + 16, &nPitch, 4);
    return payload;
}

inline std::vector<uint8_t> serializeBlockChange(int32_t x, int32_t y, int32_t z, uint8_t blockId) {
    std::vector<uint8_t> payload(13);
    uint32_t nx = htonl(static_cast<uint32_t>(x));
    uint32_t ny = htonl(static_cast<uint32_t>(y));
    uint32_t nz = htonl(static_cast<uint32_t>(z));
    std::memcpy(payload.data(), &nx, 4);
    std::memcpy(payload.data() + 4, &ny, 4);
    std::memcpy(payload.data() + 8, &nz, 4);
    payload[12] = blockId;
    return payload;
}

inline bool deserializePlayerJoin(const std::vector<uint8_t>& payload, PlayerJoinPayload& out) {
    if (payload.size() < 5) return false;
    uint32_t rawId;
    std::memcpy(&rawId, payload.data(), 4);
    out.playerId = ntohl(rawId);
    uint8_t nameLen = payload[4];
    if (payload.size() < 5u + nameLen + 20) return false;
    out.username = std::string(payload.begin() + 5, payload.begin() + 5 + nameLen);
    size_t offset = 5 + nameLen;
    uint32_t rawX, rawY, rawZ, rawYaw, rawPitch;
    std::memcpy(&rawX, payload.data() + offset, 4);
    std::memcpy(&rawY, payload.data() + offset + 4, 4);
    std::memcpy(&rawZ, payload.data() + offset + 8, 4);
    std::memcpy(&rawYaw, payload.data() + offset + 12, 4);
    std::memcpy(&rawPitch, payload.data() + offset + 16, 4);
    out.x = ntohf(rawX);
    out.y = ntohf(rawY);
    out.z = ntohf(rawZ);
    out.yaw = ntohf(rawYaw);
    out.pitch = ntohf(rawPitch);
    return true;
}

inline bool deserializePlayerLeave(const std::vector<uint8_t>& payload, PlayerLeavePayload& out) {
    if (payload.size() < 4) return false;
    uint32_t rawId;
    std::memcpy(&rawId, payload.data(), 4);
    out.playerId = ntohl(rawId);
    return true;
}

inline bool deserializePlayerPosition(const std::vector<uint8_t>& payload, PlayerPositionPayload& out) {
    if (payload.size() < 24) return false;
    uint32_t rawId;
    std::memcpy(&rawId, payload.data(), 4);
    out.playerId = ntohl(rawId);
    uint32_t rawX, rawY, rawZ, rawYaw, rawPitch;
    std::memcpy(&rawX, payload.data() + 4, 4);
    std::memcpy(&rawY, payload.data() + 8, 4);
    std::memcpy(&rawZ, payload.data() + 12, 4);
    std::memcpy(&rawYaw, payload.data() + 16, 4);
    std::memcpy(&rawPitch, payload.data() + 20, 4);
    out.x = ntohf(rawX);
    out.y = ntohf(rawY);
    out.z = ntohf(rawZ);
    out.yaw = ntohf(rawYaw);
    out.pitch = ntohf(rawPitch);
    return true;
}

inline bool deserializeBlockChange(const std::vector<uint8_t>& payload, BlockChangePayload& out) {
    if (payload.size() < 13) return false;
    uint32_t rawX, rawY, rawZ;
    std::memcpy(&rawX, payload.data(), 4);
    std::memcpy(&rawY, payload.data() + 4, 4);
    std::memcpy(&rawZ, payload.data() + 8, 4);
    out.x = static_cast<int32_t>(ntohl(rawX));
    out.y = static_cast<int32_t>(ntohl(rawY));
    out.z = static_cast<int32_t>(ntohl(rawZ));
    out.blockId = payload[12];
    return true;
}

inline std::vector<uint8_t> serializeChatMessage(const std::string& message) {
    std::vector<uint8_t> payload;
    payload.push_back(static_cast<uint8_t>(message.size()));
    payload.insert(payload.end(), message.begin(), message.end());
    return payload;
}

inline bool deserializeChatMessage(const std::vector<uint8_t>& payload, ChatMessagePayload& out) {
    if (payload.size() < 6) return false;
    uint32_t rawId;
    std::memcpy(&rawId, payload.data(), 4);
    out.playerId = ntohl(rawId);
    uint8_t nameLen = payload[4];
    if (payload.size() < 5u + nameLen + 1) return false;
    out.username = std::string(payload.begin() + 5, payload.begin() + 5 + nameLen);
    size_t offset = 5 + nameLen;
    uint8_t msgLen = payload[offset];
    if (payload.size() < offset + 1 + msgLen) return false;
    out.message = std::string(payload.begin() + offset + 1, payload.begin() + offset + 1 + msgLen);
    return true;
}

} // namespace PacketSerializer

#endif //CRAFTMINE_PACKETTYPES_H
