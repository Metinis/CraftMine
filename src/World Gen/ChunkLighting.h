#ifndef CHUNKLIGHTING_H
#define CHUNKLIGHTING_H

#include <unordered_set>

#include "Chunk.h"
#include "World.h"

namespace std {
    template <>
    struct hash<glm::ivec3> {
        size_t operator()(const glm::ivec3& v) const {
            size_t h1 = hash<int>()(v.x);
            size_t h2 = hash<int>()(v.y);
            size_t h3 = hash<int>()(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}
class ChunkLighting {
public:
    static void initialiseLight(Chunk& chunk);
    static void addSunlightValues(Chunk& chunk);
    static void addLightingValues(Chunk& chunk);
    static void recalculateLightWithNeighbours(const Chunk& chunk);
    static void propagateLight(Chunk& chunk, const glm::ivec3& startWorldPos);
    static void clearLight(Chunk& chunk, const glm::ivec3& startWorldPos);

};



#endif //CHUNKLIGHTING_H
