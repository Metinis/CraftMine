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
	RIGHT,
    DOWN,
    UP
};

class Camera
{
private:
	glm::vec3 Right, Up, WorldUp;
	void updateCameraVectors();
public:
    glm::vec3 Front;
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
