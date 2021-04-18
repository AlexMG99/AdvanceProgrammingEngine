#pragma once

#include "./glm/glm.hpp"

class Camera {
public:
	Camera();

	void Update();

	void CalculateProjViewMatrix();

public:
	float fov = 60.0;

	glm::vec3 position;
	glm::vec3 rotation;

	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projViewMatrix;

	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;

};
