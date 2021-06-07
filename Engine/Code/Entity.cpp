#include "Entity.h"
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

Entity::Entity()
{
	worldMatrix = glm::mat4(1.0);
}

Entity::Entity(glm::vec3 pos, u32 modelIndex)
{
	worldMatrix = glm::mat4(1.0);
	worldMatrix = glm::translate(worldMatrix, pos);

	this->modelIndex = modelIndex;
}

void Entity::UpdatePosition()
{
	worldMatrix = glm::mat4(1.0);
	worldMatrix = glm::translate(worldMatrix, pos);
}

void Entity::Rotate(float x, float y, float z)
{
	worldMatrix = glm::rotate(worldMatrix, x, glm::vec3(1, 0, 0));
	worldMatrix = glm::rotate(worldMatrix, y, glm::vec3(0, 1, 0));
	worldMatrix = glm::rotate(worldMatrix, z, glm::vec3(0, 0, 1));
}
