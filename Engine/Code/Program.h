#pragma once

// Buffers
struct VertexBufferAttribute
{
    u8 location;
    u8 componentCount;
    u8 offset;
};


struct VertexBufferLayout
{
    std::vector<VertexBufferAttribute>  attributes;
    u8                                  stride;
};

struct VertexShaderAttribute
{
    u8 location;
    u8 componentCount;
    std::string name;
};

struct VertexShaderLayout
{
    std::vector<VertexShaderAttribute>  attributes;
};


struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?


    VertexBufferLayout vertexBufferLayout;
    VertexShaderLayout vertexInputLayout;
};
