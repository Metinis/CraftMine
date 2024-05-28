#include "Chunk.h"
#include "World.h"
#include "Input/Input.h"
#include "ChunkMeshGeneration.h"
#include "Player/Player.h"


World::World(Camera& _camera, Scene& _scene, Player& _player) : camera(_camera), scene(_scene), player(_player)
{

    playerChunkPos = glm::ivec2((*camera.position).x / Chunk::SIZE, (*camera.position).z / Chunk::SIZE); //used for priority queues, chunks closest have priority

    //initialise threads
	chunkThread = std::thread(&World::GenerateChunkThread, this);
	chunkThread.detach();
	worldGenThread = std::thread(&World::GenerateWorldThread, this);
	worldGenThread.detach();

}

void World::GenerateChunkThread()
{
    volatile bool keepRunning = true;
	while (keepRunning)
	{
		if (!chunksToLoadData.empty())
		{
			mutexChunksToLoadData.lock();
			Chunk* chunk = chunksToLoadData.back();

			chunk->inThread = true;
			chunksToLoadData.pop_back();
			mutexChunksToLoadData.unlock();

            CheckForBlocksToBeAdded(chunk);
            chunk->LoadChunkData();

			mutexChunksToLoadData.lock();
            chunk->inThread = false;
			loadedChunks.push(std::ref(chunk));
			mutexChunksToLoadData.unlock();

		}

	}
}

void World::GenerateWorldThread()
{
    volatile bool keepRunning = true;
	while (keepRunning)
	{
		if (!chunksToGenerate.empty())
		{
			mutexChunksToGenerate.lock();
			Chunk* chunk = chunksToGenerate.back();
            chunk->inThread = true;
			chunksToGenerate.pop_back();
			mutexChunksToGenerate.unlock();

			chunk->GenBlocks();
            CheckForBlocksToBeAdded(chunk);

            mutexChunksToLoadData.lock();
			chunksToLoadData.insert(chunksToLoadData.begin(), chunk);
            //chunk->inThread = false;
			mutexChunksToLoadData.unlock();

		}
	}
}
bool World::CheckForBlocksToBeAdded(Chunk* chunk)
{
    bool hasBlocksToBeAdded = false;
    mutexBlocksToBeAddedList.lock();

    std::vector<BlocksToBeAdded> newBlocksToBeAddedList;
    for(int i = 0; i < blocksToBeAddedList.size(); i++)
    {
        BlocksToBeAdded &_blocksToBeAdded = blocksToBeAddedList[i];

        if(_blocksToBeAdded.chunkPosition == chunk->chunkPosition)
        {
            //TODO, make id more clear
            chunk->SetBlock(glm::ivec3(_blocksToBeAdded.localPosition.x, _blocksToBeAdded.localPosition.y, _blocksToBeAdded.localPosition.z), 8);
            hasBlocksToBeAdded = true;
        }
        else
        {
            newBlocksToBeAddedList.push_back(_blocksToBeAdded);
        }
    }
    blocksToBeAddedList.clear();
    for(BlocksToBeAdded _blocksToBeAdded : newBlocksToBeAddedList)
    {
        blocksToBeAddedList.push_back(_blocksToBeAdded);
    }

    mutexBlocksToBeAddedList.unlock();
    return hasBlocksToBeAdded;
}
void World::UpdateViewDistance(glm::ivec2& cameraChunkPos)
{
    //update comparator to current chunkPos
    playerChunkPos = cameraChunkPos;
    CompareChunks compareChunks;
    compareChunks._playerChunkPos = cameraChunkPos;

    std::vector<Chunk*> generateChunks; //chunks sent to the generation thread, gets sent to other thread to load buffer data
	std::vector<Chunk*> addedChunks; //chunks that already have buffer data, no need to send to generate or load
	std::vector<Chunk*> newActiveChunks; //chunks that are still in view, no need to process/remove
    std::vector<Chunk*> chunksLoading; //chunks that are in cameras view but didn't load yet, let them load, don't remove

    int min_x = (cameraChunkPos.x > viewDistance) ? (cameraChunkPos.x - viewDistance) : 0;
    int min_z = (cameraChunkPos.y > viewDistance) ? (cameraChunkPos.y - viewDistance) : 0;
    int max_x = (SIZE > cameraChunkPos.x + viewDistance) ? (cameraChunkPos.x + viewDistance) : SIZE;
    int max_z = (SIZE > cameraChunkPos.y + viewDistance) ? (cameraChunkPos.y + viewDistance) : SIZE;

	for (int x = min_x; x < max_x; x++)
	{
		for (int z = min_z; z < max_z; z++)
		{

			//if chunk doesn't exist, add it to thread queue for data generation
			if (chunks[x + SIZE * z] == nullptr || (!chunks[x + SIZE * z]->generatedBlockData && !chunks[x + SIZE * z]->inThread))
			{
				chunks[x + SIZE * z] = new Chunk(glm::vec2(x, z), *this);
                generateChunks.push_back(std::ref(chunks[x + SIZE * z]));
			}
            else if(!chunks[x + SIZE * z]->inThread && chunks[x + SIZE * z]->generatedBuffData)
            {
                newActiveChunks.push_back(std::ref(chunks[x + SIZE * z]));
            }
            else if(!chunks[x + SIZE * z]->generatedBuffData)
            {
                chunksLoading.push_back(std::ref(chunks[x + SIZE * z]));
            }

		}
	}
    //All chunks which are not in chunksLoading but in chunksToLoadData to be deleted
    //All chunks which are in activeChunk list but not in new active chunk
    for(Chunk* chunk : chunksToLoadData)
    {
        if(std::find(chunksLoading.begin(), chunksLoading.end(), chunk) == chunksLoading.end() && !chunk->inThread)
        {
            chunk->Delete();
            chunk->ClearVertexData();
        }
    }
    for(Chunk* chunk : activeChunks)
    {
        if(std::find(newActiveChunks.begin(), newActiveChunks.end(), chunk) == newActiveChunks.end() && !chunk->inThread)
        {
            chunk->Delete();
            chunk->ClearVertexData();
        }
    }
    std::sort(generateChunks.begin(), generateChunks.end(), compareChunks);
    std::sort(chunksLoading.begin(), chunksLoading.end(), compareChunks);

    mutexChunksToGenerate.lock();
    chunksToGenerate.clear();
    chunksToGenerate = generateChunks;
    mutexChunksToGenerate.unlock();

	mutexChunksToLoadData.lock();
    chunksToLoadData.clear();
    chunksToLoadData = chunksLoading;
    mutexChunksToLoadData.unlock();
	activeChunks.clear();
	activeChunks = newActiveChunks;


}


void World::GenerateChunkBuffers(std::vector<Chunk*>& addedChunks)
{
    for (Chunk* chunk : addedChunks)
    {
        if (!chunk->inThread)
        {
            //todo fix this race condition
            //chunk->chunkHasMeshes = false;
            chunk->generatedBuffData = false;
            chunk->LoadBufferData();
            chunk->generatedBuffData = true;
            if(!chunk->chunkHasMeshes)
            {
                mutexChunksToLoadData.lock();
                chunk->chunkHasMeshes = true;
                activeChunks.push_back(std::ref(chunk));
                mutexChunksToLoadData.unlock();
            }


        }
    }
    addedChunks.clear();
	
}
Chunk* World::GetChunk(int x, int y)
{
    if(x >= 0 && x < SIZE && y >= 0 && y < SIZE)
	    return chunks[x + SIZE * y];
    else
        return nullptr;
}
//used for breaking blocks
bool World::RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& currentChunk) {
    float step = 0.0f;
    float reach = 5.0f;

    while (step < reach) {
        glm::ivec3 globalPos;
        globalPos.x = static_cast<int>(std::round(rayOrigin.x + rayDirection.x * step));
        globalPos.y = static_cast<int>(std::round(rayOrigin.y + rayDirection.y * step));
        globalPos.z = static_cast<int>(std::round(rayOrigin.z + rayDirection.z * step));

        glm::ivec2 posInChunks = glm::ivec2(globalPos.x / Chunk::SIZE, globalPos.z / Chunk::SIZE);

        if(posInChunks.x >= 0 && posInChunks.x < World::SIZE && posInChunks.y >= 0 && posInChunks.y < World::SIZE){
            currentChunk = GetChunk(posInChunks.x, posInChunks.y);
            if (currentChunk != nullptr && currentChunk->generatedBlockData) {

                glm::ivec3 localPos;
                localPos.x = globalPos.x - currentChunk->chunkPosition.x * Chunk::SIZE;
                localPos.y = globalPos.y;
                localPos.z = globalPos.z - currentChunk->chunkPosition.y * Chunk::SIZE;
                if (Block::isSolid(currentChunk->GetBlockID(localPos))) {
                    result = localPos;
                    return true;
                }
            }
        }

        step+=0.01f;
    }

    return false;
}
//Used for place blocks
bool World::RaycastBlockPos(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::ivec3& result, Chunk*& currentChunk, glm::ivec3& lastEmptyPos) {

    float step = 0.0f;
    float reach = 5.0f;

    while (step < reach) {
        glm::ivec3 globalPos;
        globalPos.x = static_cast<int>(std::round(rayOrigin.x + rayDirection.x * step));
        globalPos.y = static_cast<int>(std::round(rayOrigin.y + rayDirection.y * step));
        globalPos.z = static_cast<int>(std::round(rayOrigin.z + rayDirection.z * step));

        Chunk* tempCurrentChunk = GetChunk(globalPos.x / Chunk::SIZE, globalPos.z / Chunk::SIZE);
        if (tempCurrentChunk!= nullptr && tempCurrentChunk->generatedBlockData) {

            glm::ivec3 localPos;
            localPos.x = globalPos.x - tempCurrentChunk->chunkPosition.x * Chunk::SIZE;
            localPos.y = globalPos.y;
            localPos.z = globalPos.z - tempCurrentChunk->chunkPosition.y * Chunk::SIZE;
            if (Block::isSolid(tempCurrentChunk->GetBlockID(localPos))){

                /*if(static_cast<int>(glm::abs((rayOrigin.x - (rayOrigin.x + rayDirection.x * step))) > 1) ||
                static_cast<int>(((rayOrigin.y - (rayOrigin.y + rayDirection.y * step))) < -1) ||
                static_cast<int>(glm::abs((rayOrigin.y - (rayOrigin.y + rayDirection.y * step))) > 2) ||
                static_cast<int>(glm::abs((rayOrigin.z - (rayOrigin.z + rayDirection.z * step))) > 1))
                {
                    result = localPos;
                    return true;
                }
                else
                {
                    break;
                }*/
                result = localPos;
                return true;
            }
            else
            {
                lastEmptyPos = localPos;
                currentChunk = GetChunk(globalPos.x / Chunk::SIZE, globalPos.z / Chunk::SIZE);
            }
        }
        step+=0.01f;
    }

    return false;
}
void World::PlaceBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) {
    glm::ivec3 localPos;
    glm::ivec3 lastEmptyPos;
    Chunk* currentChunk;

    // Raycast to find the block to place the face on
    if (RaycastBlockPos(rayOrigin, rayDirection, localPos, currentChunk, lastEmptyPos) &&
        !player.checkCollisionWithBlockLocal(lastEmptyPos)){

        if(currentChunk->generatedBlockData) {
            currentChunk->SetBlock(lastEmptyPos, player.getBlockID());
            std::cout<<"back updated: "<< currentChunk->chunkBools.backUpdated<<"\n";
            std::cout<<"front updated: "<< currentChunk->chunkBools.frontUpdated<<"\n";
            std::cout<<"right updated: "<< currentChunk->chunkBools.rightSideUpdated<<"\n";
            std::cout<<"left updated: "<< currentChunk->chunkBools.leftSideUpdated<<"\n";
            mutexChunksToLoadData.lock();
            chunksToLoadData.push_back(std::ref(currentChunk));
            mutexChunksToLoadData.unlock();
        }
    }
}
void World::BreakBlocks(const glm::vec3& rayOrigin, const glm::vec3& rayDirection){

    glm::ivec3 localPos;
    Chunk* currentChunk;
    if(RaycastBlockPos(rayOrigin, rayDirection, localPos, currentChunk))
    {
        currentChunk->SetBlock(localPos, 0);
        //if block broken was on border, check and update neighbouring chunk mesh
        mutexChunksToLoadData.lock();
        chunksToLoadData.push_back(currentChunk);
        mutexChunksToLoadData.unlock();
        int tempChunkX = currentChunk->chunkPosition.x;
        int tempChunkZ = currentChunk->chunkPosition.y;
        Chunk* tempChunk1 = nullptr;
        Chunk* tempChunk2 = nullptr;
        if(localPos.x == 0 || localPos.x == Chunk::SIZE-1)
        {
            tempChunkX = (localPos.x == 0) ? currentChunk->chunkPosition.x-1 : currentChunk->chunkPosition.x+1;
            tempChunk1 = GetChunk(tempChunkX, currentChunk->chunkPosition.y);
        }
        if(localPos.z == 0 || localPos.z == Chunk::SIZE-1)
        {
            tempChunkZ = (localPos.z == 0) ? currentChunk->chunkPosition.y-1 : currentChunk->chunkPosition.y+1;
            tempChunk2 = GetChunk(currentChunk->chunkPosition.x, tempChunkZ);
        }
        //2 temp chunks just in case we are in corner
        if(tempChunk1 != nullptr && tempChunk1->generatedBlockData)
        {
            mutexChunksToLoadData.lock();
            chunksToLoadData.push_back(std::ref(tempChunk1));
            mutexChunksToLoadData.unlock();
        }
        if(tempChunk2 != nullptr && tempChunk2->generatedBlockData)
        {
            mutexChunksToLoadData.lock();
            chunksToLoadData.push_back(std::ref(tempChunk2));
            mutexChunksToLoadData.unlock();
        }
    }

}

void World::LoadThreadDataToMain()
{
    if (!loadedChunks.empty())
    {
        std::vector<Chunk*> addedChunks;
        mutexChunksToLoadData.lock();
        for (int i = 0; i < loadedChunks.size(); i++)
        {
            Chunk* chunk = loadedChunks.front();
            if(!chunk->inThread) {
                chunk->sortTransparentMeshData(); //sort transparent faces before rendering
                if (CheckForBlocksToBeAdded(chunk)){
                    loadedChunks.pop();
                    chunksToLoadData.push_back(chunk);
                }
                else if(!chunk->getIsAllSidesUpdated()){
                    loadedChunks.pop();
                    ChunkMeshGeneration::UpdateNeighbours(*chunk);
                    addedChunks.push_back(std::ref(chunk));
                }
                else{
                    addedChunks.push_back(std::ref(chunk));
                    loadedChunks.pop();
                }

            }
        }
        mutexChunksToLoadData.unlock();
        if (!addedChunks.empty()){
            GenerateChunkBuffers(addedChunks); //adds to active chunks
        }

    }
}
void World::sortChunks(){
    //sort active chunks by farthest from the player in front -> for transparency
    CompareChunks compareChunks;
    compareChunks._playerChunkPos = playerChunkPos;
    std::sort(activeChunks.begin(), activeChunks.end(), compareChunks);
}
void World::sortTransparentFaces() {
    if(player.chunkPosition.x > 0 && player.chunkPosition.x < World::SIZE && player.chunkPosition.y > 0 && player.chunkPosition.y < World::SIZE) {
        Chunk *currentChunk = GetChunk(playerChunkPos.x, playerChunkPos.y);
        if(currentChunk != nullptr) {

            //when moving inbetween chunks, sort surrounding chunks
            if (player.chunkPosition.x != currentChunk->chunkPosition.x ||
                player.chunkPosition.y != currentChunk->chunkPosition.y) {
                for (int x = (int) player.chunkPosition.x - 2;
                     x < (int) player.chunkPosition.x + 2 && x > 0 && x < World::SIZE; x++) {
                    for (int z = (int) player.chunkPosition.y - 2;
                         z < (int) player.chunkPosition.y + 2 && z > 0 && z < World::SIZE; z++) {
                        Chunk *currentChunkToSort = GetChunk(x, z);
                        if (currentChunkToSort != nullptr && !currentChunkToSort->inThread &&
                            currentChunkToSort->generatedBuffData) {
                            //world->mutexChunksToLoadData.lock();
                            loadedChunks.push(currentChunkToSort); //loadedchunks sorts each chunk transparent face
                            //world->mutexChunksToLoadData.unlock();
                        }
                    }
                }

            }
                //else sort the current chunk the player is in every time you move a block
            else {
                if (!currentChunk->inThread &&
                    glm::round(player.lastPosition) != glm::round(player.position) && currentChunk->generatedBuffData)
                    //only sort if block pos has changes hence round
                {
                    //world->mutexChunksToLoadData.lock();
                    loadedChunks.push(currentChunk); //loadedchunks sorts each chunk transparent face
                    //world->mutexChunksToLoadData.unlock();
                }
            }
            player.chunkPosition = currentChunk->chunkPosition;
        }
    }
}
//default way to render
void World::renderChunks()
{
    for (Chunk* chunk : activeChunks)
    {
        if(chunk->chunkHasMeshes && chunk->mesh->loadedData && chunk->transparentMesh->loadedData&& chunk->mesh != nullptr &&
           chunk->transparentMesh != nullptr){
            scene.renderMesh(*chunk->mesh, *scene.shader);
            glDepthMask(GL_FALSE);
            scene.renderMesh(*chunk->transparentMesh, *scene.transparentShader);
            glDepthMask(GL_TRUE);
        }
    }
}
//optional way to render with dedicated shader (used for shadow map depth)
void World::renderChunks(Shader& shader)
{
    for (Chunk* chunk : activeChunks)
    {
        if(chunk->chunkHasMeshes && chunk->mesh->loadedData && chunk->transparentMesh->loadedData && &shader != nullptr && chunk->mesh != nullptr &&
        chunk->transparentMesh != nullptr){
            scene.renderMesh(*chunk->mesh, shader);

            scene.renderMesh(*chunk->transparentMesh, shader);
        }
    }
}
void World::update()
{
    sortTransparentFaces();

    scene.updateShaders();
    //changes global texture every second that passes
    scene.changeGlobalTexture();

    LoadThreadDataToMain();

    sortChunks();
}

