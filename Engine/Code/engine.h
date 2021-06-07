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
#include "Resources/FrameBuffer.h"
#include "Resources/Enviroment.h"

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
    Light() {};
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


enum Mode : int
{
    ForwardRendering,
    DeferredRendering,
    Mode_Count
};

enum WaterScenePart {
    Reflection,
    Refraction
};

struct WaterShader {

    GLuint rtReflection = 0;
    GLuint rtRefraction = 0;
    GLuint rtReflectDepth = 0;
    GLuint rtRefractDepth = 0;

    unsigned int fboReflection;
    unsigned int fboRefraction;

    Entity* waterPlaneEntity;

    bool active = true;

    // Water parameters
    vec2 waveLength = vec2(1.0);
    vec2 waveStrength = vec2(0.02);
    float turbidityDistance = 2.5;
    float shineDamper = 20.0;
    float reflectivity = 0.6;

    vec2 speed = vec2(0.1);
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
    i32 albedoTextureIdx = -1;
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
    float time = 0.0f;

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

    // Water
    WaterShader waterEffect;



    // program indices
    u32 texturedGeometryProgramIdx;
    u32 texturedMeshProgramIdx;
    u32 waterProgramIdx;
    u32 clippingProgramIdx;
    u32 simpleProgramIdx;
    u32 irradianceShaderIdx;
    u32 skyboxProgramId;
    u32 skyboxForwardProgramId;
    u32 forwardProgramIdx;
    GLuint lightingPassProgram;
    GLuint bakeCubeMapProgram;

    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;
    u32 normalWaterIdx;
    u32 dudvWaterIdx;

    //mesh 
    u32 quadModel;
    u32 cubeModel;

    u32 waterPlane;

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

    //Enviroments
    Enviroment enviroment;

    // Light Sun
    Light sun;

    //
    u32     globalParamsOffset;
    u32     globalParamsSize;

    GLint uniformBlockAligment;
    unsigned int gBuffer;
    u32 gDiffuse, gDepth, gNormals, gPosition;

    Entity patrick;

    //To bake HDR to Cubemap
    FBO captureFBO;
    RBO captureRBO;

    // Lighting pass Shader parameters
    int renderMode = 0;
    float F0 = 0.5;
    float minFresnel = 0.1;
    float maxFresnel = 1.0;
    int skyboxIdx = 0;
    std::vector<Light> lights;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

void PassWaterScene(App* app, WaterScenePart part);

u32 LoadTexture2D(App* app, const char* filepath);

void InitGBuffer(App* app);

void RenderScene(App* app, Camera camera, Program& program);

// Water Shader
void createBuffers(App* app, WaterShader& wShader);
void createTextureAttachment(App* app, GLuint& id, unsigned int fboBuffer);
void createDepthTextureAttachment(App* app, GLuint& id, unsigned int fboBuffer);

void RenderInGBuffer(App* app);

void RenderSkybox(App* app, Camera* cam);

void RenderSkyboxForward(App* app, Camera* cam);


void RenderWater(App* app);

void LightingPass(App* app);

void ForwardRender(App* app);


