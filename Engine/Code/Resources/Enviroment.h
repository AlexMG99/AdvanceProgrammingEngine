#pragma once
#include "Component.h"
#include "Texture.h"

struct TextureCube
{
	unsigned int handle; 
	void Init();
	void Bind(int i);
};

struct App;
class Texture;

class Enviroment : public Component
{
public:
	Enviroment();
	~Enviroment();
	void Init();
	void CreateEnviromentFromTexture(App* app, Texture& tex);
	void BindEnviroment(int i = 0);
	virtual void HandleResourcesAboutToDie() override {};
	//Texture* texture = nullptr;
	bool isSetUp = false;
	TextureCube enviromentMap;
	TextureCube irradianceMap;
};

void GenerateEnviromentCaptureFrames(App* app);