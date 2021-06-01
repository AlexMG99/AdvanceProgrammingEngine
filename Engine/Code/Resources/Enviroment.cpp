#include "Enviroment.h"
#include "../engine.h"

void GenerateEnviromentCaptureFrames(App* app)
{
	glGenFramebuffers(1, &app->captureFBO.handle);
	glGenRenderbuffers(1, &app->captureRBO.handle);

	glBindFramebuffer(GL_FRAMEBUFFER, app->captureFBO.handle);
	glBindRenderbuffer(GL_RENDERBUFFER, app->captureRBO.handle);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, app->captureRBO.handle);
}

Enviroment::Enviroment()
{
	
}

Enviroment::~Enviroment()
{

}

void Enviroment::Init()
{
	if (!isSetUp)
	{
		enviromentMap.Init();
		isSetUp = true;
	}
}

void Enviroment::CreateEnviromentFromTexture(App* app, Texture& tex)
{
	Init();

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// convert HDR equirectangular environment map to cubemap equivalent
	Program& equirectangularToCubemapShader = app->programs[app->bakeCubeMapProgram];
	equirectangularToCubemapShader.Bind();
	equirectangularToCubemapShader.glUniformInt("equirectangularMap", 0);
	equirectangularToCubemapShader.glUniformMatrix4("projection", captureProjection);
	tex.Bind(0);

	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	app->captureFBO.Bind();
	app->captureRBO.Bind();
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectangularToCubemapShader.glUniformMatrix4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, enviromentMap.handle, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Model& cube = app->models[app->cubeModel];
		cube.Render(app, equirectangularToCubemapShader);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Enviroment::BindEnviroment(int i)
{
	enviromentMap.Bind(i);
}

void TextureCube::Init()
{
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
	for (unsigned int i = 0; i < 6; ++i)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
			512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void TextureCube::Bind(int i)
{
	glActiveTexture(GL_TEXTURE0 + i);
	glBindTexture(GL_TEXTURE_2D, handle);
}
