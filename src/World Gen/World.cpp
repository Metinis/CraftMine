#include "Chunk.h"
#include "World.h"


World::World(Camera& _camera) : camera(_camera)
{
    shader = new Shader("../resources/shader/shader.vs", "../resources/shader/shader.fs");
    transparentShader = new Shader("../resources/shader/transparent.vs", "../resources/shader/transparent.fs");
    texture = new Texture("../resources/texture/terrain1.png");
	model = glm::mat4(1.0f);
	view = glm::mat4(1.0f);
	proj = glm::perspective(glm::radians(65.0f), 16.0f / 9.0f, 0.1f, 10000.0f);
    playerChunkPos = glm::ivec2(camera.Position.x / Chunk::SIZE, camera.Position.z / Chunk::SIZE); //used for priority queues, chunks closest have priority
	chunkThread = std::thread(&World::GenerateChunkThread, this);
	chunkThread.detach();
	worldGenThread = std::thread(&World::GenerateWorldThread, this);
	worldGenThread.detach();

	BindPrograms();
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
void World::BindPrograms()
{
    shader->use();
    shader->setMat4("model", model);
    shader->setMat4("projection", proj);
    texture->Bind();

    transparentShader->use();
    transparentShader->setMat4("model", model);
    transparentShader->setMat4("projection", proj);
    texture->Bind();
}
void World::ChangeGlobalTexture()
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
}
void World::RenderWorld(Camera _camera)
{
    //changes global texture every second that passes
    int currentTime = (int)glfwGetTime();
    if(currentTime != lastTime)
    {
        ChangeGlobalTexture();
        lastTime = currentTime;
    }

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

	view = _camera.GetViewMatrix();
    shader->use();
	shader->setMat4("view", view);
    transparentShader->use();
    transparentShader->setMat4("view", view);

    //sort active chunks by farthest from the player in front -> for transparency
    CompareChunks compareChunks;
    compareChunks._playerChunkPos = playerChunkPos;
    std::sort(activeChunks.begin(), activeChunks.end(), compareChunks);

	for (Chunk* chunk : activeChunks)
	{
		if(chunk->generatedBuffData && !chunk->inThread)
		chunk->RenderChunk();
	}

}

