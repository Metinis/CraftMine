#include "Chunk.h"
#include "World.h"
#include "Input/MouseInput.h"
#include "ChunkMeshGeneration.h"


World::World(Camera& _camera) : camera(_camera)
{
    shader = new Shader("../resources/shader/shader.vs", "../resources/shader/shader.fs");

    outlineShader = new Shader("../resources/shader/OutlineShader.vs", "../resources/shader/OutlineShader.fs");

    transparentShader = new Shader("../resources/shader/transparent.vs", "../resources/shader/transparent.fs");

    texture = new Texture("../resources/texture/terrain1.png");

	model = glm::mat4(1.0f);
	view = glm::mat4(1.0f);
	proj = glm::perspective(glm::radians(65.0f), 16.0f / 9.0f, 0.25f, 10000.0f);

    outlineShader->use();
    outlineShader->setMat4("model", model);
    outlineShader->setMat4("projection", proj);

    playerChunkPos = glm::ivec2(camera.Position.x / Chunk::SIZE, camera.Position.z / Chunk::SIZE); //used for priority queues, chunks closest have priority

    //initialise threads
	chunkThread = std::thread(&World::GenerateChunkThread, this);
	chunkThread.detach();
	worldGenThread = std::thread(&World::GenerateWorldThread, this);
	worldGenThread.detach();

	LoadShader(shader);
    LoadShader(transparentShader);
}

[[noreturn]] void World::GenerateChunkThread()
{
	while (true)
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
			loadedChunks.push(std::ref(chunk));
            chunk->inThread = false;
			mutexChunksToLoadData.unlock();
		}
	}
}

[[noreturn]] void World::GenerateWorldThread()
{
	while (true)
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
void World::CheckForBlocksToBeAdded(Chunk* chunk)
{
    mutexBlocksToBeAddedList.lock();

    std::vector<BlocksToBeAdded> newBlocksToBeAddedList;
    for(int i = 0; i < blocksToBeAddedList.size(); i++)
    {
        BlocksToBeAdded &_blocksToBeAdded = blocksToBeAddedList[i];

        if(_blocksToBeAdded.chunkPosition == chunk->chunkPosition)
        {
            chunk->SetBlock(glm::ivec3(_blocksToBeAdded.localPosition.x, _blocksToBeAdded.localPosition.y, _blocksToBeAdded.localPosition.z), 8);
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
}
void World::UpdateViewDistance(glm::ivec2 cameraChunkPos)
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
			chunk->LoadBufferData();
			chunk->generatedBuffData = true;
			activeChunks.push_back(std::ref(chunk));
		}
	}
	addedChunks.clear();
	
}
Chunk* World::GetChunk(int x, int y)
{
	return chunks[x + SIZE * y];
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

        currentChunk = GetChunk(globalPos.x / Chunk::SIZE, globalPos.z / Chunk::SIZE);
        if (currentChunk != nullptr && currentChunk->generatedBlockData) {

            glm::ivec3 localPos;
            localPos.x = globalPos.x - currentChunk->chunkPosition.x * Chunk::SIZE;
            localPos.y = globalPos.y;
            localPos.z = globalPos.z - currentChunk->chunkPosition.y * Chunk::SIZE;
            if (currentChunk->GetBlockID(localPos) != 0) {
                result = localPos;
                return true;
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
            if (tempCurrentChunk->GetBlockID(localPos) != 0) {

                if(static_cast<int>(glm::abs((rayOrigin.x - (rayOrigin.x + rayDirection.x * step))) > 1) ||
                static_cast<int>(glm::abs((rayOrigin.y - (rayOrigin.y + rayDirection.y * step))) > 1) ||
                static_cast<int>(glm::abs((rayOrigin.z - (rayOrigin.z + rayDirection.z * step))) > 1))
                {
                    result = localPos;
                    return true;
                }
                else
                {
                    break;
                }
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
    if (RaycastBlockPos(rayOrigin, rayDirection, localPos, currentChunk, lastEmptyPos)) {

        if(currentChunk->generatedBlockData) {
            currentChunk->SetBlock(lastEmptyPos, 3);
            mutexChunksToLoadData.lock();
            chunksToLoadData.push_back(currentChunk);
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
            chunksToLoadData.push_back(tempChunk1);
            mutexChunksToLoadData.unlock();
        }
        if(tempChunk2 != nullptr && tempChunk2->generatedBlockData)
        {
            mutexChunksToLoadData.lock();
            chunksToLoadData.push_back(tempChunk2);
            mutexChunksToLoadData.unlock();
        }
    }

}
void World::LoadShader(Shader* shader)
{
    shader->use();
    shader->setMat4("model", model);
    shader->setMat4("projection", proj);
    shader->setVec3("cameraPos", camera.Position);
    shader->setFloat("fogStart", ((viewDistance - (viewDistance/3)) < viewDistance - 1 ? (viewDistance - (viewDistance/3)) : 0) * Chunk::SIZE);
    shader->setFloat("fogEnd", (viewDistance-1) * Chunk::SIZE);
    shader->setVec3("fogColor", glm::vec3(0.55f, 0.75f, 1.0f));
    texture->Bind();
}
void World::ChangeGlobalTexture()
{
    //Updates every second
    int currentTime = (int)glfwGetTime();
    if(currentTime != lastTime)
    {
        if(lastTexture < 5) {
            lastTexture++;
        }
        else {
            lastTexture = 1;
        }
        std::stringstream path;
        path << "../resources/texture/terrain" << lastTexture << ".png";
        std::string texturePath = path.str();
        texture->setTexture(texturePath.c_str());

        lastTime = currentTime;
    }

}
void World::RenderBlockOutline()
{
    glm::ivec3 result;
    Chunk* currentChunk;
    if(RaycastBlockPos(camera.Position, camera.Front, result, currentChunk)){
        glm::ivec3 globalPos = glm::vec3(result.x + currentChunk->chunkPosition.x * Chunk::SIZE, result.y, result.z + currentChunk->chunkPosition.y * Chunk::SIZE);
        if(globalPos != lastOutlinePos)
        {
            UpdateOutlineBuffers(globalPos);
            DrawOutline();
        }
        else
        {
            DrawOutline();
        }
        lastOutlinePos = globalPos;

    }

}
void World::UpdateOutlineBuffers(glm::ivec3& globalPos){
    std::vector<glm::vec3> vertices = Block::GetOutline(globalPos);
    std::vector<GLuint> indices;

    if(outlineVAO != nullptr)
    {
        outlineVAO->Delete();
        delete outlineVAO;
        outlineVAO = nullptr;
    }
    if(outlineVBO != nullptr)
    {
        outlineVBO->Delete();
        delete outlineVBO;
        outlineVBO = nullptr;
    }
    if(outlineIBO != nullptr)
    {
        outlineIBO->Delete();
        delete outlineIBO;
        outlineIBO = nullptr;
    }
    //Calculates the indices for the vertices, reusing a method for chunks
    int _indexCount = 0;
    for (int i = 0; i < 6; i++)
    {
        indices.push_back(3 + _indexCount);
        indices.push_back(0 + _indexCount);
        indices.push_back(0 + _indexCount);
        indices.push_back(1 + _indexCount);
        indices.push_back(1 + _indexCount);
        indices.push_back(2 + _indexCount);
        indices.push_back(2 + _indexCount);
        indices.push_back(3 + _indexCount);

        _indexCount += 4;	//uses element index, since each face only has 4 indices, we increment this everytime any block face is added
    }

    outlineVAO = new VAO();

    outlineVBO = new VBO(vertices);
    outlineVBO->Bind();

    outlineVAO->LinkToVAO(outlineShader->getAttribLocation("aPos"), 3, *outlineVBO);

    outlineVBO->Unbind();
    outlineVAO->Unbind();

    outlineIBO = new IBO(indices);
}

void World::DrawOutline() const
{
    int indexCount = 48;
    outlineShader->use();
    outlineVAO->Bind();
    outlineIBO->Bind();
    glDrawElements(GL_LINES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
    outlineVAO->Unbind();
    outlineIBO->Unbind();
}
void World::UpdateShaders()
{
    shader->use();
    shader->setVec3("cameraPos", camera.Position);
    transparentShader->use();
    transparentShader->setVec3("cameraPos", camera.Position);
    view = camera.GetViewMatrix();
    outlineShader->use();
    outlineShader->setMat4("view", view);
    shader->use();
    shader->setMat4("view", view);
    transparentShader->use();
    transparentShader->setMat4("view", view);

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
            addedChunks.push_back(chunk);
            loadedChunks.pop();
        }
        mutexChunksToLoadData.unlock();
        if (!addedChunks.empty())
            GenerateChunkBuffers(addedChunks); //adds to active chunks
    }
}
void World::SortAndRenderChunks()
{
    //sort active chunks by farthest from the player in front -> for transparency
    CompareChunks compareChunks;
    compareChunks._playerChunkPos = playerChunkPos;
    std::sort(activeChunks.begin(), activeChunks.end(), compareChunks);

    for (Chunk* chunk : activeChunks)
    {
        if(chunk->chunkHasMeshes)
            chunk->RenderChunk();
    }
}
void World::RenderWorld()
{
    UpdateShaders();
    //changes global texture every second that passes
    ChangeGlobalTexture();

    LoadThreadDataToMain();

    SortAndRenderChunks();

    RenderBlockOutline();
}

