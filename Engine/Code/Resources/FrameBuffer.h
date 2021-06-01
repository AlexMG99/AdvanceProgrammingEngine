#pragma once

struct FBO
{
	unsigned int handle;
	void Bind();
	static void UnBind();
};

struct RBO
{
	unsigned int handle;
	void Bind();
};