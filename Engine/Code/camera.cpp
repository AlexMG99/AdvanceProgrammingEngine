#include "camera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

Camera::Camera()
{
	position = glm::vec3(0.0f, 0.0f, 3.0f);
	rotation = glm::vec3(0.0f, 0.0f, 0.0f);

	projMatrix = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);

}

void Camera::Update()
{
	CalculateProjViewMatrix();
}

void Camera::CalculateProjViewMatrix()
{
	glm::vec3 direction;
	direction.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
	direction.y = sin(glm::radians(rotation.x));
	direction.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));

	// Calculate 
	front = glm::normalize(direction);
	glm::vec3 globalUp = glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::normalize(glm::cross(globalUp, front));
	up = glm::cross(direction, right);

	viewMatrix = glm::lookAt(position, position + front, up);
	// viewMatrix = glm::lookAt(position, glm::vec3(0,0,0), globalUp);

	projViewMatrix = projMatrix * viewMatrix;
}
