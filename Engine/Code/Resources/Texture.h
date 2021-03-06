#pragma once

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include <glad/glad.h>
#include <string>

typedef glm::vec3  vec3;
typedef glm::ivec2 ivec2;
typedef int          i32;

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
    bool isHDR = false;
};


class Texture
{
    public:
    GLuint      handle;
    std::string filepath;
    void Bind(int i);
};

Image LoadImage(const char* filename);
void FreeImage(Image image);
GLuint CreateTexture2DFromImage(Image image);
GLuint CreateTexture2DFromHDRImage(Image image);