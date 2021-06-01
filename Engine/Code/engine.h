//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include <glad/glad.h>
#include <vector>
#include "assimp_model_loading.h"
#include "Program.h"
#include "camera.h"
#include "Entity.h"
#include "Resources/Texture.h"

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

struct Buffer {
    GLuint  handle;
    GLenum  type;
    u32     size;
    u32     head;
    void*   data;
};

enum LightType
{
    LightType_Directional,
    LightType_Point
};

struct Light
{
    Light(LightType t, vec3 c, vec3 d, vec3 p)
    {
        type = t;
        color = c;
        direction = d;
        position = p;
    };
    LightType type;
    vec3 color;
    vec3 direction;
    vec3 position;
};


enum Mode
{
    Mode_TexturedQuad,
    Mode_Count
};

struct OpenGLInfo
{
    std::string version;
    std::string renderer;
    std::string vendor;
    std::string shadingLanguage;
    std::string* extensions;
};

struct Vao
{
    GLuint handle;
    GLuint programHandle;
};

// FBX components
struct Model 
{
    u32                 meshIdx;
    std::vector<u32>    materialIdx;

    void Render(App* app, Program& program);
};

struct Submesh
{
    VertexBufferLayout  vertexBufferLayout;
    std::vector<float>  vertices;
    std::vector<u32>    indices;
    u32                 vertexOffset;
    u32                 indexOffset;

    std::vector<Vao>    vaos;
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    GLuint vertexBufferHandle;
    GLuint indexBufferHandle;

    void SetupBuffers();
};

struct Material
{
    std::string name;
    vec3 albedo;
    vec3 emissive;
    f32 smoothness;
    u32 albedoTextureIdx;
    u32 emissiveTextureIdx;
    u32 specularTextureIdx;
    u32 normalsTextureIdx;
    u32 bumpTextureIdx;
};

// Image Example
struct VertexV3V2
{
    glm::vec3 pos;
    glm::vec2 uv;
};

const VertexV3V2 vertices[] = {
    { glm::vec3(-0.5, -0.5, 0.0), glm::vec2(0.0, 0.0)}, // bottom-left vertex
    { glm::vec3(0.5, -0.5, 0.0), glm::vec2(1.0, 0.0)}, // bottom-left vertex
    { glm::vec3(0.5, 0.5, 0.0), glm::vec2(1.0, 1.0)}, // bottom-left vertex
    { glm::vec3(-0.5, 0.5, 0.0), glm::vec2(0.0, 1.0)}, // bottom-left vertex
};

const u16 indices[] = {
    0, 1, 2,
    0, 2, 3
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize;

    std::vector<Texture>    textures;
    std::vector<Material>   materials;
    std::vector<Mesh>       meshes;
    std::vector<Model>      models;
    std::vector<Program>    programs;
    std::vector<Entity>     entities;
    // program indices
    u32 texturedGeometryProgramIdx;
    u32 texturedMeshProgramIdx;
    u32 waterProgramIdx;
    u32 simpleProgramIdx;
    GLuint lightingPassProgram;
    GLuint skyBoxProgram;

    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    //mesh 
    u32 quadModel;
    u32 cubeModel;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;
    GLuint textureMeshProgram_uTexture;
    u32 skyTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;

    // OpenGL information
    OpenGLInfo glInfo;

    // Camera
    Camera* cam;
    int cameraSpeed = 10;

    // Buffer
    Buffer cbuffer;
    Buffer deferredbuffer;

    u32     globalParamsOffset;
    u32     globalParamsSize;

    GLint uniformBlockAligment;
    unsigned int gBuffer;
    u32 gDiffuse, gDepth, gNormals, gPosition;

    int renderMode = 0;

    std::vector<Light> lights;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

u32 LoadTexture2D(App* app, const char* filepath);

void InitGBuffer(App* app);