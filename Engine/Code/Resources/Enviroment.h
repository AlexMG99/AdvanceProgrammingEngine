#pragma once
#include "Component.h"
#include "Texture.h"

struct TextureCube
{
	unsigned int handle; 
	void Init(int size);
	void Bind(int i);
};

struct App;
class Texture;

class Enviroment : public Component
{
public:
	Enviroment();
	~Enviroment();
	void Init(App* app);
	void CreateEnviromentFromTexture(App* app, Texture& tex);
	void BindEnviroment(int i = 0);
	void BindIrradiaceMap(int i = 0);

	virtual void HandleResourcesAboutToDie() override {};
	//Texture* texture = nullptr;
	bool isSetUp = false;

private:
	TextureCube enviromentMap;
	TextureCube irradianceMap;
};

void GenerateEnviromentCaptureFrames(App* app);