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
	void updateCameraVectors();
public:
    glm::vec3 Right{};
    glm::vec3 Front{};

	glm::vec3 Up{}, WorldUp{};
	float Yaw;
	float Pitch;
    glm::vec3 position{};
	Camera();

	glm::mat4 getViewMatrix() const;
	void processMouseMovement(float xoffset, float yoffset);
    void updatePosition(glm::vec3 lastPlayerPos, glm::vec3 playerPos, double alpha);

};
#endif
