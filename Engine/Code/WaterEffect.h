#include "engine.h"


class WaterEffect {
	WaterEffect() {};

	void createBuffers(App* app);


	void createTextureAttachment(App* app, GLuint id, unsigned int fboBuffer);
	void createDepthTextureAttachment(App* app, GLuint id, unsigned int fboBuffer);

	void renderWater(App* app, Camera* camera);

	//void passWaterScene(App* app, Camera* camera, GLenum colorAttachment, WaterScenePart part);

	void cleanUp();

	GLuint rtReflection = 0;
	GLuint rtRefraction = 0;
	GLuint rtReflectDepth = 0;
	GLuint rtRefractDepth = 0;

	unsigned int fboReflection;
	unsigned int fboRefraction;

};
