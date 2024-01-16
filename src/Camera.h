#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum cameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 50.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

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

	Camera(glm::vec3 position = glm::vec3(16.0f, 20.0f, 16.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f)) {
		Position = position;
		WorldUp = up;
		Front = front;
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
		Yaw = YAW;
		Pitch = PITCH;
		movementSpeed = SPEED;
		MouseSensitivity = SENSITIVITY;
	}

	glm::mat4 GetViewMatrix();

	void ProcessKeyboardMovement(cameraMovement dir, float deltaTime);
	void ProcessMouseMovement(float xoffset, float yoffset);

};
#endif
