#include "Chunk.h"
#include "World.h"


World::World(Camera& _camera) : camera(_camera)
{
	shader = new Shader("${CMAKE_CURRENT_SOURCE_DIR}/../../resources/shader/shader.vs", "${CMAKE_CURRENT_SOURCE_DIR}/../../resources/shader/shader.fs");
	texture = new Texture("${CMAKE_CURRENT_SOURCE_DIR}/../../resources/texture/atlas.png");
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
			Chunk* chunk = chunksToLoadData.front();

			chunk->inThread = true;
			chunksToLoadData.pop();
			mutexChunksToLoadData.unlock();

                chunk->LoadChunkData();

            chunk->inThread = false;

			mutexChunksToLoadData.lock();
			loadedChunks.push(std::ref(chunk));
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
			Chunk* chunk = chunksToGenerate.front();
            chunk->inThread = true;
			chunksToGenerate.pop();
			mutexChunksToGenerate.unlock();

			chunk->GenBlocks();

            mutexChunksToLoadData.lock();
			chunksToLoadData.push(chunk);
			mutexChunksToLoadData.unlock();
		}
	}
}

void World::UpdateViewDistance(glm::ivec2 cameraChunkPos)
{
    playerChunkPos = cameraChunkPos;
    CompareChunks compareChunks;
    compareChunks._playerChunkPos = cameraChunkPos;

	int min_x;
	int min_z;
	int max_x;
	int max_z;

	//CompareChunks.cameraPos = cameraChunkPos;
    std::vector<Chunk*> generateChunks;
	std::vector<Chunk*> addedChunks;
	std::vector<Chunk*> newActiveChunks;
	if (cameraChunkPos.x > viewDistance)
	{
		min_x = cameraChunkPos.x - viewDistance;
	}
	else
	{
		min_x = 0;
	}
	if (cameraChunkPos.y > viewDistance)
	{
		min_z = cameraChunkPos.y - viewDistance;
	}
	else
	{
		min_z = 0;
	}

	if (SIZE > cameraChunkPos.x + viewDistance)
	{
		max_x = cameraChunkPos.x + viewDistance;
	}
	else
	{
		max_x = SIZE;
	}
	if (SIZE > cameraChunkPos.y + viewDistance)
	{
		max_z = cameraChunkPos.y + viewDistance;
	}
	else
	{
		max_z = SIZE;
	}

	for (int x = min_x; x < max_x; x++)
	{
		for (int z = min_z; z < max_z; z++)
		{
			
			//if chunk doesn't exist, add it to thread queue for data generation
			if (chunks[x + SIZE * z] == nullptr)
			{
				chunks[x + SIZE * z] = new Chunk(glm::vec2(x, z), *this);
                generateChunks.push_back(std::ref(chunks[x + SIZE * z]));
			}
			else if(!chunks[x + SIZE * z]->inThread)
			{
				//check if chunk is already in active list, if it isn't add to the addedChunks list for updating meshes
				if (std::find(activeChunks.begin(), activeChunks.end(), chunks[x + SIZE * z]) == activeChunks.end())	//not in list
				{
                    addedChunks.push_back(std::ref(chunks[x + SIZE * z]));
				}
				else
				{
					newActiveChunks.push_back(std::ref(chunks[x + SIZE * z]));
				}

			}
		}
	}
    std::sort(generateChunks.begin(), generateChunks.end(), compareChunks);
    std::sort(addedChunks.begin(), addedChunks.end(), compareChunks);
    for(Chunk* chunk : generateChunks) {
        mutexChunksToGenerate.lock();
        chunksToGenerate.push(chunk);
        mutexChunksToGenerate.unlock();
    }
    for(Chunk* chunk : addedChunks)
    {
        mutexChunksToLoadData.lock();
        chunksToLoadData.push(chunk);
        mutexChunksToLoadData.unlock();
    }
	/*for (Chunk* chunk : activeChunks)
	{
		if (std::find(addedChunks.begin(), addedChunks.end(), chunk) == addedChunks.end())
		{
			mutexChunksToDelete.lock();
			chunksToDelete.push_back(chunk);
			mutexChunksToDelete.unlock();
		}
	}*/
	//need to delete buffers of unused chunks
	//mutexChunksToLoadData.lock();
	//for (Chunk* c : activeChunks)
	//{
	//	if (std::find(newActiveChunks.begin(), newActiveChunks.end(), c) == newActiveChunks.end())
	//	{
	//		if(c->generated && !c->inThread)
	//		c->Delete();
	//	}

	//}
	//mutexChunksToLoadData.unlock();
	mutexChunksToLoadData.lock();
	activeChunks.clear();
	activeChunks = newActiveChunks;
	mutexChunksToLoadData.unlock();

	//need to update already existing chunks too neighbouring them
	
	//GenerateChunkBuffers(addedChunks); -> recalculate faces instead -> too much ram to store vertex data
	
}


void World::GenerateChunkBuffers(std::vector<Chunk*>& addedChunks)
{
	for (Chunk* chunk : addedChunks)
	{
		if (!chunk->inThread)
		{
			chunk->ReloadBufferData();
			chunk->generated = true;
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
}
void World::RenderWorld(Camera _camera)
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
			GenerateChunkBuffers(addedChunks);
	}

	view = _camera.GetViewMatrix();
	shader->setMat4("view", view);

	for (Chunk* chunk : activeChunks)
	{
		if(chunk->generated)
		chunk->RenderChunk();
	}
}

