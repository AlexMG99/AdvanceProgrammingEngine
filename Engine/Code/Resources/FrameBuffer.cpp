#include "FrameBuffer.h"
#include "glad/glad.h"

void FBO::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void FBO::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RBO::Bind()
{
	glBindRenderbuffer(GL_RENDERBUFFER,handle);
}
