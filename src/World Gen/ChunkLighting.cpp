#include "ChunkLighting.h"
void ChunkLighting::initialiseLight(Chunk& chunk) {
    for (int y = Chunk::HEIGHT - 1; y >= 0; y--) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            for (int x = 0; x < Chunk::SIZE; x++) {
                constexpr std::array<unsigned char, 4> rgbaLight = {16, 16, 16, 2}; // Default (no light)
                chunk.SetBlockLighting(glm::ivec3(x, y, z), rgbaLight);
            }
        }
    }
    addSunlightValues(chunk);
}
void ChunkLighting::addSunlightValues(Chunk& chunk) {
    for (int z = 0; z < Chunk::SIZE; z++) {
        for (int x = 0; x < Chunk::SIZE; x++) {
            bool isBlocked = false;
            for (int y = Chunk::HEIGHT - 1; y >= 0; y--) {

                const glm::ivec3 pos(x, y, z);

                if(isBlocked) {
                    continue;
                }

                constexpr std::array<unsigned char, 4> rgbaLight = {16, 16, 16, 16}; // Default (no light)
                chunk.SetBlockLighting(glm::ivec3(x, y, z), rgbaLight);

                if(!Block::isTransparent(chunk.GetBlockID(pos))) {
                    isBlocked = true;
                }

            }
        }
    }
    for (int z = 0; z < Chunk::SIZE; z++) {
        for (int x = 0; x < Chunk::SIZE; x++) {
            bool isBlocked = false;
            for (int y = Chunk::HEIGHT - 1; y >= 0; y--) {

                const glm::ivec3 pos(x, y, z);

                if(!Block::isTransparent(chunk.GetBlockID(pos))) {
                    isBlocked = true;
                }

                if(isBlocked) {
                    //need to check bordering light values here
                    constexpr std::array<glm::ivec3, 6> directions = {
                        glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0), glm::ivec3(0, 1, 0),
                        glm::ivec3(0, -1, 0), glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)
                    };
                    auto worldPos = glm::ivec3(pos.x + chunk.chunkPosition.x * Chunk::SIZE, pos.y, pos.z + chunk.chunkPosition.y * Chunk::SIZE);
                    for (const auto& direction : directions) {

                        const glm::ivec3 neighborWorldPos = worldPos + direction;
                        if(neighborWorldPos.y >= Chunk::HEIGHT || neighborWorldPos.y < 0) {
                            continue;
                        }
                        const auto neighborChunkXZ = glm::ivec2(
                        (static_cast<float>(neighborWorldPos.x) / Chunk::SIZE),
                        (static_cast<float>(neighborWorldPos.z) / Chunk::SIZE)
                        );
                        const Chunk* neighborChunk = chunk.world.GetChunk(neighborChunkXZ.x, neighborChunkXZ.y);
                        if(!neighborChunk) {
                            constexpr std::array<unsigned char, 4> rgbaLight = {16, 16, 16, 16}; // Default (no light)
                            chunk.SetBlockLighting(glm::ivec3(x, y, z), rgbaLight);
                            continue;
                        }

                        glm::ivec3 neighborLocalPos;
                        neighborLocalPos.x = neighborWorldPos.x - neighborChunkXZ.x * Chunk::SIZE;
                        neighborLocalPos.y = neighborWorldPos.y;
                        neighborLocalPos.z = neighborWorldPos.z - neighborChunkXZ.y * Chunk::SIZE;

                        std::array<unsigned char, 4> neighborLight = neighborChunk->getBlockLightValue(neighborLocalPos);

                        if(neighborLight[3] > chunk.getBlockLightValue(pos)[3]) {
                            chunk.SetBlockLighting(pos, {neighborLight[0], neighborLight[1], neighborLight[2]
                                , static_cast<unsigned char>(neighborLight[3] - 1)});
                        }
                    }

                }

            }
        }
    }
}
void ChunkLighting::addLightingValues(Chunk& chunk)  {

    for (int z = 0; z < Chunk::SIZE; z++) {
        for (int x = 0; x < Chunk::SIZE; x++) {
            for (int y = Chunk::HEIGHT - 1; y >= 0; y--) {

                const glm::ivec3 pos(x, y, z);
                // Check for light-emitting blocks
                if (chunk.GetBlockID(pos) == 45) {
                    constexpr std::array<unsigned char, 4> rgbaLight = {16, 16, 16, 16};
                    chunk.SetBlockLighting(pos, rgbaLight);
                    auto worldPos = glm::ivec3(pos.x + chunk.chunkPosition.x * Chunk::SIZE, pos.y, pos.z + chunk.chunkPosition.y * Chunk::SIZE);
                    propagateLight(chunk, worldPos);
                }
            }
        }
    }
}
void ChunkLighting::recalculateLightWithNeighbours(const Chunk& chunk) {

    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            Chunk* currentChunk = chunk.world.GetChunk(chunk.chunkPosition.x + i, chunk.chunkPosition.y + j);
            if(!currentChunk) {
                std::cout<<"err\n";
                continue;
            }
            //clear current chunk light
            initialiseLight(*currentChunk);
        }
    }
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            Chunk* currentChunk = chunk.world.GetChunk(chunk.chunkPosition.x + i, chunk.chunkPosition.y + j);
            if(!currentChunk) {
                std::cout<<"err\n";
                continue;
            }
            addLightingValues(*currentChunk);
            WorldThreading::updateLoadData(currentChunk);
        }
    }

}
void ChunkLighting::propagateLight(Chunk& chunk, const glm::ivec3& startWorldPos){
    constexpr std::array<glm::ivec3, 6> directions = {
        glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0), glm::ivec3(0, 1, 0),
        glm::ivec3(0, -1, 0), glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)
    };

    std::queue<glm::ivec3> queue;
    std::unordered_set<glm::ivec3> visited;

    queue.push(startWorldPos);
    visited.insert(startWorldPos);

    while (!queue.empty()) {
        glm::ivec3 currentWorldPos = queue.front();
        queue.pop();

        const auto currentChunkXZ = glm::ivec2(currentWorldPos.x / Chunk::SIZE, currentWorldPos.z / Chunk::SIZE);
        Chunk* currentChunk = chunk.world.GetChunk(currentChunkXZ.x, currentChunkXZ.y);

        if (!currentChunk) {
            std::cout << "\nnullptr at chunk: " << currentChunkXZ.x << "x " << currentChunkXZ.y << "z";
            continue;
        }

        glm::ivec3 currentLocalPos;
        currentLocalPos.x = currentWorldPos.x - currentChunkXZ.x * Chunk::SIZE;
        currentLocalPos.y = currentWorldPos.y;
        currentLocalPos.z = currentWorldPos.z - currentChunkXZ.y * Chunk::SIZE;

        std::array<unsigned char, 4> currentLight = currentChunk->getBlockLightValue(currentLocalPos);
        if (currentLight[3] == 0) {
            continue;
        }

        for (const auto& direction : directions) {
            glm::ivec3 neighborWorldPos = currentWorldPos + direction;
            if (visited.find(neighborWorldPos) != visited.end()) continue;

            const auto neighborChunkXZ = glm::ivec2(
                (static_cast<float>(neighborWorldPos.x) / Chunk::SIZE),
                (static_cast<float>(neighborWorldPos.z) / Chunk::SIZE)
            );
            Chunk* neighborChunk = chunk.world.GetChunk(neighborChunkXZ.x, neighborChunkXZ.y);

            if (!neighborChunk) {
                std::cout << "\nNeighbor nullptr at chunk: " << neighborChunkXZ.x << "x " << neighborChunkXZ.y << "z";
                continue;
            }

            glm::ivec3 neighborLocalPos;
            neighborLocalPos.x = neighborWorldPos.x - neighborChunkXZ.x * Chunk::SIZE;
            neighborLocalPos.y = neighborWorldPos.y;
            neighborLocalPos.z = neighborWorldPos.z - neighborChunkXZ.y * Chunk::SIZE;

            unsigned char newLight = currentLight[3] - 1;

            // Stop propagation if the new light is less than 1
            if (newLight < 1) {
                continue;
            }

            std::array<unsigned char, 4> neighborLight = neighborChunk->getBlockLightValue(neighborLocalPos);

            // Stop propagation if the new light is not stronger than the neighbor's current light
            if (newLight <= neighborLight[3]) {
                continue;
            }

            // Update light and continue propagation
            std::array<unsigned char, 4> newLightValue = {
                currentLight[0],
                currentLight[1],
                currentLight[2],
                newLight
            };

            neighborChunk->SetBlockLighting(neighborLocalPos, newLightValue);
            WorldThreading::updateLoadData(neighborChunk);
            queue.push(neighborWorldPos);
            visited.insert(neighborWorldPos);
        }
    }

}
void ChunkLighting::clearLight(Chunk& chunk, const glm::ivec3& startWorldPos) {
    constexpr std::array<glm::ivec3, 6> directions = {
        glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0), glm::ivec3(0, 1, 0),
        glm::ivec3(0, -1, 0), glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)
    };

    std::queue<glm::ivec3> queue;
    std::unordered_set<glm::ivec3> visited;

    queue.push(startWorldPos);
    visited.insert(startWorldPos);

    while (!queue.empty()) {
        glm::ivec3 currentWorldPos = queue.front();
        queue.pop();

        const auto currentChunkXZ = glm::ivec2(
            std::floor(static_cast<float>(currentWorldPos.x) / Chunk::SIZE),
            std::floor(static_cast<float>(currentWorldPos.z) / Chunk::SIZE)
        );
        Chunk* currentChunk = chunk.world.GetChunk(currentChunkXZ.x, currentChunkXZ.y);

        if (!currentChunk) continue;

        glm::ivec3 currentLocalPos;
        currentLocalPos.x = currentWorldPos.x - currentChunkXZ.x * Chunk::SIZE;
        currentLocalPos.y = currentWorldPos.y;
        currentLocalPos.z = currentWorldPos.z - currentChunkXZ.y * Chunk::SIZE;

        std::array<unsigned char, 4> currentLight = currentChunk->getBlockLightValue(currentLocalPos);

        if (currentLight[3] == 0) continue;

        std::array<unsigned char, 4> clearedLight = {0, 0, 0, 0};
        currentChunk->SetBlockLighting(currentLocalPos, clearedLight);

        for (const auto& direction : directions) {
            glm::ivec3 neighborWorldPos = currentWorldPos + direction;
            if (visited.find(neighborWorldPos) != visited.end()) continue;

            const auto neighborChunkXZ = glm::ivec2(
                (static_cast<float>(neighborWorldPos.x) / Chunk::SIZE),
                (static_cast<float>(neighborWorldPos.z) / Chunk::SIZE)
            );
            Chunk* neighborChunk = chunk.world.GetChunk(neighborChunkXZ.x, neighborChunkXZ.y);

            if (!neighborChunk) continue;

            glm::ivec3 neighborLocalPos;
            neighborLocalPos.x = neighborWorldPos.x - neighborChunkXZ.x * Chunk::SIZE;
            neighborLocalPos.y = neighborWorldPos.y;
            neighborLocalPos.z = neighborWorldPos.z - neighborChunkXZ.y * Chunk::SIZE;

            std::array<unsigned char, 4> neighborLight = neighborChunk->getBlockLightValue(neighborLocalPos);


            // If the neighbors light is lower than the current light it should be cleared
            WorldThreading::updateLoadData(neighborChunk);
            if (neighborLight[3] > 0 && neighborLight[3] <= currentLight[3]) {
                queue.push(neighborWorldPos);
                visited.insert(neighborWorldPos);
            }
        }
    }

    addLightingValues(chunk);
}