#pragma once
#include "Component.h"
#include "Texture.h"

class TextureCube;
class Enviroment : public Component
{
public:
	Enviroment();
	~Enviroment();
	virtual void HandleResourcesAboutToDie() override;
	Texture* texture = nullptr;
	bool needsPorcessing = false;
	TextureCube* enviromentMap = nullptr;
	TextureCube* irradianceMap = nullptr;
};