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
    float MouseSensitivity;
	glm::vec3 Up, WorldUp;
	void updateCameraVectors();
public:
    glm::vec3 Right;
    glm::vec3 Front;
	float Yaw;
	float Pitch;
    glm::vec3 *position = nullptr;
	Camera();

	glm::mat4 GetViewMatrix();
	void ProcessMouseMovement(float xoffset, float yoffset);

};
#endif
