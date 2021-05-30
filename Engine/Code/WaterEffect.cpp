#include "WaterEffect.h"
#include "buffer_management.h"

void WaterEffect::createBuffers(App* app)
{
	// Create fboReflection
	glGenFramebuffers(1, &fboReflection);
	glBindFramebuffer(GL_FRAMEBUFFER, fboReflection);
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT };
	glDrawBuffers(2, attachments);

	// Assign textures
	createTextureAttachment(app, rtReflection, fboReflection);
	createDepthTextureAttachment(app, rtReflectDepth, fboReflection);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create fboRefraction
	glGenFramebuffers(1, &fboRefraction);
	glBindFramebuffer(GL_FRAMEBUFFER, fboRefraction);
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT };
	glDrawBuffers(2, attachments);

	// Assign textures
	createTextureAttachment(app, rtRefraction, fboRefraction);
	createDepthTextureAttachment(app, rtRefractDepth, fboRefraction);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WaterEffect::createTextureAttachment(App* app, GLuint id, unsigned int fboBuffer)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboBuffer, 0);
}

void WaterEffect::createDepthTextureAttachment(App* app, GLuint id, unsigned int fboBuffer)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, app->displaySize.x, app->displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fboBuffer, 0);
}

void WaterEffect::renderWater(App* app, Camera* camera)
{
	// Reflection
	glBindFramebuffer(GL_FRAMEBUFFER, fboReflection);

	Camera reflectionCamera = *camera;
	reflectionCamera.position.y = -reflectionCamera.position.y;
	reflectionCamera.right = -reflectionCamera.right;
	//reflectionCamera.viewportWidth = reflectionCamera.viewportWidth;
	//reflectionCamera.viewportHeight = reflectionCamera.viewportHeight;
	reflectionCamera.CalculateProjViewMatrix();

	passWaterScene(app, &reflectionCamera, GL_COLOR_ATTACHMENT0, WaterScenePart::Reflection);
	//passBackground(&reflectionCamera, GL_COLOR_ATTACHMENT0); // Quizas es passar la textura de color?

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Refraction
	glBindFramebuffer(GL_FRAMEBUFFER, fboRefraction);
	Camera refractionCamera = *camera;
	refractionCamera.position.y = -refractionCamera.position.y;
	refractionCamera.right = -refractionCamera.right;
	//reflectionCamera.viewportWidth = reflectionCamera.viewportWidth;
	//reflectionCamera.viewportHeight = reflectionCamera.viewportHeight;
	refractionCamera.CalculateProjViewMatrix();

	passWaterScene(app, &refractionCamera, GL_COLOR_ATTACHMENT0, WaterScenePart::Refraction);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void WaterEffect::passWaterScene(App* app, Camera* camera, GLenum colorAttachment, WaterScenePart part)
{
	glDrawBuffer(colorAttachment);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CLIP_DISTANCE0);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Program& waterProgram = app->programs[app->waterProgramIdx];
	glUseProgram(waterProgram.handle);

	// If it is Binded El buffer esta mal, no se como va
	glm::mat4x4 viewMatrix = camera->viewMatrix;

}

void WaterEffect::cleanUp()
{
	glDeleteFramebuffers(1, &fboReflection);
	glDeleteFramebuffers(1, &fboRefraction);
	glDeleteTextures(1, &rtRefraction);
	glDeleteTextures(1, &rtReflection);
	glDeleteTextures(1, &rtReflectDepth);
	glDeleteTextures(1, &rtRefractDepth);
}
