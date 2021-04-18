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

}
