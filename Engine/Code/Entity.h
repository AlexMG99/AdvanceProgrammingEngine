#pragma once
#include "glm/glm.hpp"

typedef unsigned int           u32;

class Entity
{
public:
	Entity();
	Entity(glm::vec3 pos, u32 modelIndex =0);

	glm::mat4 worldMatrix;
	glm::vec3 pos;

	u32 modelIndex = 0;
	u32 localParamsOffset = 0;
	u32 localParamsSize = 0;
};