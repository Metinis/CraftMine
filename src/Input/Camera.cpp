#include "Camera.h"

Camera::Camera()
{
    Position = glm::vec3(16.0f, 20.0f, 16.0f);
    WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    Front = glm::vec3(0.0f, 0.0f, -1.0f);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
    Yaw = -90.0f;
    Pitch = 0.0f;
    movementSpeed = 25.0f;
    MouseSensitivity = 0.1f;
}
glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(Position, Position + Front, WorldUp);
}
void Camera::ProcessKeyboardMovement(cameraMovement dir, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;
	if (dir == cameraMovement::FORWARD)
		Position += glm::normalize(glm::vec3(Front.x, 0, Front.z)) * velocity;
	if (dir == cameraMovement::BACKWARD)
		Position -= glm::normalize(glm::vec3(Front.x, 0, Front.z)) * velocity;
	if (dir == cameraMovement::LEFT)
		Position -= Right * velocity;
	if (dir == cameraMovement::RIGHT)
		Position += Right * velocity;
    if (dir == cameraMovement::DOWN)
        Position.y -= velocity;
    if (dir == cameraMovement::UP)
        Position.y += velocity;
}
void Camera::ProcessMouseMovement(float xoffset, float yoffset)
{
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (Pitch > 89.0f)
		Pitch = 89.0f;
	if (Pitch < -89.0f)
		Pitch = -89.0f;

	// update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}
void Camera::updateCameraVectors()
{
	// calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	// also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));
}
