#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <vector>

#include "Camera.h"
#include "Chunk.h"
#include "World.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, bool* wireframe, bool* keyProccessed);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


World* world;
Camera camera;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	bool wireframe = false;
	bool keyProcessed = false;


	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CraftMine", nullptr, nullptr);

	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// Disable depth writes (for correct transparency sorting)
    //glDepthMask(GL_FALSE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	camera.Position = glm::vec3(World::SIZE*Chunk::SIZE / 2, Chunk::HEIGHT, World::SIZE*Chunk::SIZE / 2);
	world = new World(camera);
	

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	
	//render loop
	glm::ivec2 lastChunkPos = glm::ivec2(camera.Position.x / Chunk::SIZE, camera.Position.z / Chunk::SIZE);
	glm::ivec2 newChunkPos = glm::ivec2(camera.Position.x / Chunk::SIZE, camera.Position.z / Chunk::SIZE);

	int updateingInt = 1; //world->viewDistance/2; //so it doesn't update every chunk
	world->UpdateViewDistance(newChunkPos);

	while (!glfwWindowShouldClose(window))
	{
		auto currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window, &wireframe, &keyProcessed);
		newChunkPos = glm::ivec2(camera.Position.x / Chunk::SIZE, camera.Position.z / Chunk::SIZE);
		
		if (std::abs(newChunkPos.x-lastChunkPos.x) >= updateingInt || std::abs(newChunkPos.y - lastChunkPos.y) >= updateingInt)
		{
			lastChunkPos = newChunkPos;
			std::cout << newChunkPos.x << "x " << newChunkPos.y << "z \n";
			world->UpdateViewDistance(newChunkPos);
		}
		glClearColor(0.55f, 0.75f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		world->RenderWorld(camera);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, bool* wireframe, bool* keyProccessed)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS && !*wireframe && !*keyProccessed)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		*wireframe = true;
		*keyProccessed = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS && *wireframe && !*keyProccessed)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		*wireframe = false;
		*keyProccessed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_RELEASE)
	{
		*keyProccessed = false;
	}

    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
        // Get the primary monitor
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

        // Get the video mode of the primary monitor
        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

        // Switch the window to fullscreen mode
        glfwSetWindowMonitor(window, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
        std::cout<<world->blocksToBeAddedList[world->blocksToBeAddedList.size() - 1].localPosition.x << "x " << world->blocksToBeAddedList[0].localPosition.y<< "y " << world->blocksToBeAddedList[0].localPosition.z<< "z ";
    }
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(cameraMovement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(cameraMovement::RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	auto xpos = static_cast<float>(xposIn);
	auto ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}