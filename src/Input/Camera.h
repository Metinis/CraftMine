#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <vector>

enum cameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class Camera
{
private:
	glm::vec3 Right, Up, Front, WorldUp;
	void updateCameraVectors();
public:
	
	float Yaw;
	float Pitch;
	float movementSpeed;
	float MouseSensitivity;
	glm::vec3 Position;

	Camera();

	glm::mat4 GetViewMatrix();

	void ProcessKeyboardMovement(cameraMovement dir, float deltaTime);
	void ProcessMouseMovement(float xoffset, float yoffset);

};
#endif
