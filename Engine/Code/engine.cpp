//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include "glm/gtc/type_ptr.hpp"
#include "buffer_management.h"
#include "Core.h"

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};

        if(!image.isHDR)
            tex.handle = CreateTexture2DFromImage(image);
        else
            tex.handle = CreateTexture2DFromHDRImage(image);

        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

void InitGBuffer(App* app)
{
    //Create gBuffer ==
    glGenFramebuffers(1, &app->gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, app->gBuffer);
    glEnable(GL_DEPTH_TEST);
    
    glGenTextures(1, &app->gDiffuse);
    glBindTexture(GL_TEXTURE_2D, app->gDiffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, app->gDiffuse, 0);

    glGenTextures(1, &app->gNormals);
    glBindTexture(GL_TEXTURE_2D, app->gNormals);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, app->gNormals, 0);

    glGenTextures(1, &app->gPosition);
    glBindTexture(GL_TEXTURE_2D, app->gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, app->gPosition, 0);

    glGenTextures(1, &app->gDepth);
    glBindTexture(GL_TEXTURE_2D, app->gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, app->displaySize.x, app->displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, app->gDepth, 0);

    GLenum gBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (gBufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        //TODO put log
    }

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    app->lightingPassProgram = LoadProgram(app, "deferred.glsl", "TEXTURED_GEOMETRY");
   

    Program& geometryProgram = app->programs[app->lightingPassProgram];
   
    geometryProgram.Bind();
    geometryProgram.glUniformInt("gDiffuse", 0);
    geometryProgram.glUniformInt("gNormal", 1);
    geometryProgram.glUniformInt("gDepth", 2);
    geometryProgram.glUniformInt("gPosition", 3);
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    // Try finding a vao for this submesh/program
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
        if (submesh.vaos[i].programHandle == program.handle)
            return submesh.vaos[i].handle;
    
    GLuint vaoHandle = 0;

    // Create a new vao for this submesh/program
    {
        glGenVertexArrays(1, &vaoHandle);
        glBindVertexArray(vaoHandle);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

        bool attributeWasLinked = false;

        // We have to link all vertex inputs attributes to attribute in the vertex buffer
        for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
        {
            for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
            {
                if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
                {
                    const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                    const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                    const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset;
                    const u32 stride = submesh.vertexBufferLayout.stride;
                    glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                    glEnableVertexAttribArray(index);

                    attributeWasLinked = true;
                    break;
                }
            }

            assert(attributeWasLinked);
        }

        glBindVertexArray(0);
    }

    // Store it in the list of vaos for this submesh
    Vao vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;

}

void Init(App* app)
{
    GLint maxUniformBufferSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAligment);

    glGenBuffers(1, &app->cbuffer.handle);
    glBindBuffer(GL_UNIFORM_BUFFER, app->cbuffer.handle);
    glBufferData(GL_UNIFORM_BUFFER, maxUniformBufferSize, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &app->deferredbuffer.handle);
    glBindBuffer(GL_UNIFORM_BUFFER, app->deferredbuffer.handle);
    glBufferData(GL_UNIFORM_BUFFER, maxUniformBufferSize, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Generate water shader buffers texture
    createBuffers(app, app->waterEffect);

    // Create Camera
    app->cam = new Camera(60.0f, 0.1f, 1000.0f, (float)(app->displaySize.x/app->displaySize.y));

    // Load Program
    app->texturedMeshProgramIdx = LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
    Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
    app->textureMeshProgram_uTexture = glGetUniformLocation(texturedMeshProgram.handle, "uTexture");

    //Load Water Shader
    app->waterProgramIdx = LoadProgram(app, "waterShader.glsl", "TEXTURED_GEOMETRY");
    app->skyboxProgramId = LoadProgram(app, "skybox.glsl", "SKYBOX");
    Program& skyboxProgram = app->programs[app->skyboxProgramId];
    skyboxProgram.Bind();
    skyboxProgram.glUniformInt("environmentMap", 0);
    app->clippingProgramIdx = LoadProgram(app, "shadersClipping.glsl", "TEXTURED_GEOMETRY");

    u32 patrickID = LoadModel(app, "Patrick/Patrick.obj");
    u32 planeID = LoadModel(app, "Plane/plane.obj");
    u32 sphereID = LoadModel(app, "Sphere/sphere.obj");
    u32 mountainModel = LoadModel(app, "Mountain/mountain.obj");
    u32 waterModel = LoadModel(app, "Water/water.obj");

    app->quadModel = CreatePlane(app);
    app->cubeModel =  CreateCube(app);
    
    GenerateEnviromentCaptureFrames(app);
    app->bakeCubeMapProgram = LoadProgram(app, "hdrToCubemap.glsl", "");
    app->simpleProgramIdx = LoadProgram(app, "Simple.glsl", "");
    app->skyTexture = LoadTexture2D(app, "kiara_1_dawn_1k.hdr");
    app->enviroment.CreateEnviromentFromTexture(app, app->textures[app->skyTexture]);

    //Textures ==
    app->normalWaterIdx = LoadTexture2D(app, "Water/normalMap.png");
    app->dudvWaterIdx = LoadTexture2D(app, "Water/waterDUDV.png");

    app->mode = Mode_WaterShader;

   // app->mode = Mode_WaterShader;

    //Entities =====
    Entity entity;
    //app->entities.push_back(Entity(vec3(0.0, 0.0, 0.0), patrickID));

    //entity = Entity(vec3(0.0, 0.0, 0.0), app->cubeModel);
    //entity.Rotate(0, -30, 0);
    //app->entities.push_back(entity);

    if (app->mode == Mode_WaterShader)
    {
        // Mountain and water assets
        entity = Entity(vec3(0.0, 0.0, 0.0), mountainModel);
        app->entities.push_back(entity);

        app->waterEffect.waterPlaneEntity = new Entity(vec3(5.0, 0.0, 0.0), waterModel);
        //app->entities.push_back(*app->waterEffect.waterPlaneEntity);
    }
    else
    {
        entity = Entity(vec3(4.0, 0.0, 3.0), patrickID);
        entity.Rotate(0, -30, 0);
        app->entities.push_back(entity);

        entity = Entity(vec3(-4.0, 0.0, 3.0), patrickID);
        entity.Rotate(0, 30, 0);
        app->entities.push_back(entity);

        app->entities.push_back(Entity(vec3(0.0, -4.0, 2.5), planeID));
    }

    InitGBuffer(app);

    // Create Light
    Light light = Light(LightType_Directional, vec3(1.0,1,1), vec3(0, -1,0 ), vec3(10, 10, 10));
    app->lights.push_back(light);
    

    light = Light(LightType_Point, vec3(1.0, .0, .0), vec3(0, -1, 0), vec3(-10, 0, 3));
    app->lights.push_back(light);
    entity = Entity(vec3(10, 10, 10), sphereID);
    app->entities.push_back(entity);
    
    light = Light(LightType_Point, vec3(0.0, 1.0, .0), vec3(0, -1, 0), vec3(3, 2, 3));
    app->lights.push_back(light);
    entity = Entity(vec3(3, 2, 3), sphereID);
    app->entities.push_back(entity);
    
    light = Light(LightType_Point, vec3(0.0, .0, 1.0), vec3(0, -1, 0), vec3(-5, 5, 0));
    app->lights.push_back(light);
    entity = Entity(vec3(-5, 5, 0), sphereID);
    app->entities.push_back(entity);
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);

    ImGui::Separator();

    ImGui::Text("Camera options:"); ImGui::SetNextItemWidth(100);

    ImGui::DragFloat("X    ", &app->cam->position.x); ImGui::SameLine(); ImGui::SetNextItemWidth(100);
    ImGui::DragFloat("Y    ", &app->cam->position.y); ImGui::SameLine(); ImGui::SetNextItemWidth(100);
    ImGui::DragFloat("Z    ", &app->cam->position.z); ImGui::SetNextItemWidth(100);

    ImGui::DragFloat("Pitch", &app->cam->rotation.x); ImGui::SameLine(); ImGui::SetNextItemWidth(100);
    ImGui::DragFloat("Yaw  ", &app->cam->rotation.y); ImGui::SameLine(); ImGui::SetNextItemWidth(100);
    ImGui::DragFloat("Roll ", &app->cam->rotation.z); ImGui::SetNextItemWidth(100);

    ImGui::DragFloat("FOV    ", &app->cam->fov); ImGui::SameLine(); ImGui::SetNextItemWidth(100);
    ImGui::DragFloat("Near    ", &app->cam->farPlane); ImGui::SameLine(); ImGui::SetNextItemWidth(100);
    ImGui::DragFloat("Far    ", &app->cam->nearPlane);

    ImGui::Separator();

    // Lights
    ImGui::Text("Directional Light"); ImGui::SetNextItemWidth(100);

    ImGui::DragFloat("Dir X    ", &app->lights[0].direction.x, 0.05); ImGui::SameLine(); ImGui::SetNextItemWidth(100);
    ImGui::DragFloat("Dir Y    ", &app->lights[0].direction.y, 0.05); ImGui::SameLine(); ImGui::SetNextItemWidth(100);
    ImGui::DragFloat("Dir Z    ", &app->lights[0].direction.z, 0.05); ImGui::SetNextItemWidth(100);

    ImGui::Text("Color: "); ImGui::SetNextItemWidth(100);

    ImGui::DragFloat("R    ", &app->lights[0].color.x, 0.01, 0, 1.0); ImGui::SameLine(); ImGui::SetNextItemWidth(100);
    ImGui::DragFloat("G    ", &app->lights[0].color.y, 0.01, 0, 1.0); ImGui::SameLine(); ImGui::SetNextItemWidth(100);
    ImGui::DragFloat("B    ", &app->lights[0].color.z, 0.01, 0, 1.0); ImGui::SetNextItemWidth(100);

    // Todo apply changes to camera when properties modified

    const char* items[] = { "Final", "Normal", "Depth", "Position"};
    ImGui::Combo("Render mode", &app->renderMode, items, IM_ARRAYSIZE(items));

    ImGui::Separator();


    if (ImGui::CollapsingHeader("OpenGL Info"))
    {
        std::string openGL = "OpenGL version: ";
        ImGui::Text((openGL + app->glInfo.version).c_str());
        openGL = "OpenGL renderer: ";
        ImGui::Text((openGL + app->glInfo.renderer).c_str());
        openGL = "OpenGL vendor: ";
        ImGui::Text((openGL + app->glInfo.vendor).c_str());
        openGL = "OpenGL GLSL version: ";
        ImGui::Text((openGL + app->glInfo.shadingLanguage).c_str());
        openGL = "OpenGL extensions: ";

        GLint numExtension;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtension);
        for (int i = 0; i < numExtension; i++)
        {
            openGL += app->glInfo.extensions[i] + "\n                   ";
        }
        ImGui::Text(openGL.c_str());
    }



    ImGui::End();
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
    // Camera Input

    if (app->input.mouseButtons[1] == ButtonState::BUTTON_PRESSED) 
    {
       
        app->cam->rotation.x -= app->input.mouseDelta.y;
        app->cam->rotation.y += app->input.mouseDelta.x;

        if (app->cam->rotation.x > 89.0f)
            app->cam->rotation.x = 89.0f;
        if (app->cam->rotation.x < -89.0f)
            app->cam->rotation.x = -89.0f;

    }

    if (app->input.IsKeyPressed(K_A))
        app->cam->position += app->cameraSpeed * app->deltaTime * app->cam->right;
    else if (app->input.IsKeyPressed(K_D))
        app->cam->position -= app->cameraSpeed * app->deltaTime * app->cam->right;
    //else if (app->input.IsKeyPressed(K_W))
    //    app->cam->position.y += app->cameraSpeed * app->deltaTime;
    //else if (app->input.IsKeyPressed(K_S))
    //    app->cam->position.y -= app->cameraSpeed * app->deltaTime;
    else if (app->input.IsKeyPressed(K_W))
        app->cam->position += app->cameraSpeed * app->deltaTime * app->cam->front;
    else if (app->input.IsKeyPressed(K_S))
        app->cam->position -= app->cameraSpeed * app->deltaTime * app->cam->front;

    // Camera update
    app->cam->Update();
}

void Render(App* app)
{
    switch (app->mode)
    {
        case Mode_TexturedQuad:
            {
                RenderInGBuffer(app);

                LightingPass(app);
                
            }
            break;
        case Mode_WaterShader:
            {

                Camera refractionCamera = *app->cam;
                refractionCamera.position.y = -refractionCamera.position.y;
                refractionCamera.rotation.x = -refractionCamera.rotation.x;
                refractionCamera.CalculateProjViewMatrix();

                PassWaterScene(app, refractionCamera, WaterScenePart::Reflection);

                PassWaterScene(app, refractionCamera, WaterScenePart::Refraction);

                RenderInGBuffer(app);

                LightingPass(app);
            }
        break;

        default:
            
            break;
    }
}


void PassWaterScene(App* app, Camera camera, WaterScenePart part)
{
    if(part == WaterScenePart::Reflection)
        glBindFramebuffer(GL_FRAMEBUFFER, app->waterEffect.fboReflection);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, app->waterEffect.fboRefraction);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CLIP_DISTANCE0);

    // Draw function
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    RenderSkybox(app, &camera);

    Program& clippingShader = app->programs[app->clippingProgramIdx];
    clippingShader.Bind();
    clippingShader.glUniformMatrix4("viewMatrixReflection", camera.viewMatrix);

    if (part == WaterScenePart::Reflection)
    {
        clippingShader.glUniformVec4("clippingPlane", glm::vec4(0, 1, 0, 0));
    }
    else
    {
        clippingShader.glUniformVec4("clippingPlane", glm::vec4(0, -1, 0, 0));
    }

    RenderScene(app, camera, clippingShader);

    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Deferred Shading ======
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CLIP_DISTANCE0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Mesh::SetupBuffers()
{
    u32 vertexBufferSize = 0;
    u32 indexBufferSize = 0;

    for (u32 i = 0; i < submeshes.size(); ++i)
    {
        vertexBufferSize += submeshes[i].vertices.size() * sizeof(float);
        indexBufferSize += submeshes[i].indices.size() * sizeof(u32);
    }

    glGenBuffers(1, &vertexBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, NULL, GL_STATIC_DRAW);

    u32 indicesOffset = 0;
    u32 verticesOffset = 0;

    for (u32 i = 0; i < submeshes.size(); ++i)
    {
        const void* verticesData = submeshes[i].vertices.data();
        const u32   verticesSize = submeshes[i].vertices.size() * sizeof(float);
        glBufferSubData(GL_ARRAY_BUFFER, verticesOffset, verticesSize, verticesData);
        submeshes[i].vertexOffset = verticesOffset;
        verticesOffset += verticesSize;

        const void* indicesData = submeshes[i].indices.data();
        const u32   indicesSize = submeshes[i].indices.size() * sizeof(u32);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indicesOffset, indicesSize, indicesData);
        submeshes[i].indexOffset = indicesOffset;
        indicesOffset += indicesSize;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Model::Render(App* app, Program& program)
{
    Mesh& meshQuad = app->meshes[this->meshIdx];
    for (u32 i = 0; i < meshQuad.submeshes.size(); ++i)
    {
       
        GLuint vao = FindVAO(meshQuad, i, program);
        glBindVertexArray(vao);

        Submesh& submeshQuad = meshQuad.submeshes[0];
        glDrawElements(GL_TRIANGLES, submeshQuad.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submeshQuad.indexOffset);
    }
}

void RenderScene(App* app, Camera cam, Program& program)
{
    glBindBuffer(GL_UNIFORM_BUFFER, app->cbuffer.handle);
    app->cbuffer.head = 0;
    app->cbuffer.data = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);

    // Local Params
    for (int i = 0; i < app->entities.size(); ++i)
    {
        AlignHead(app->cbuffer, app->uniformBlockAligment); // TODO set the 0 value to an uniformBlockAligment 
        Entity& entity = app->entities[i];
        glm::mat4    world = entity.worldMatrix;
        glm::mat4    worldViewProjection = cam.projViewMatrix * entity.worldMatrix;

        entity.localParamsOffset = app->cbuffer.head;
        PushMat4(app->cbuffer, world);
        PushMat4(app->cbuffer, worldViewProjection);
        entity.localParamsSize = app->cbuffer.head - entity.localParamsOffset;

        glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->cbuffer.handle, entity.localParamsOffset, entity.localParamsSize);

        Model& model = app->models[entity.modelIndex];
        Mesh& mesh = app->meshes[model.meshIdx];

        for (u32 i = 0; i < mesh.submeshes.size(); ++i)
        {
            GLuint vao = FindVAO(mesh, i, program);
            glBindVertexArray(vao);

            u32 submeshMaterialIdx = model.materialIdx[i];
            Material& submeshMaterial = app->materials[submeshMaterialIdx];

            if (submeshMaterial.albedoTextureIdx != -1)
            {
                program.glUniformInt("hasTexture", 1);
                glUniform1i(app->textureMeshProgram_uTexture, 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
            }
            else
            {
                program.glUniformInt("hasTexture", 0);
                program.glUniformVec3("color", submeshMaterial.albedo);
            }

            Submesh& submesh = mesh.submeshes[i];
            glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
        }
    }
}

void createBuffers(App* app, WaterShader& wShader)
{
    // Create fboReflection
    glGenFramebuffers(1, &wShader.fboReflection);
    glBindFramebuffer(GL_FRAMEBUFFER, wShader.fboReflection);
    glEnable(GL_DEPTH_TEST);

    // Assign textures
    createTextureAttachment(app, wShader.rtReflection, wShader.fboReflection);
    createDepthTextureAttachment(app, wShader.rtReflectDepth, wShader.fboReflection);

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT };
    glDrawBuffers(2, attachments);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create fboRefraction
    glGenFramebuffers(1, &wShader.fboRefraction);
    glBindFramebuffer(GL_FRAMEBUFFER, wShader.fboRefraction);
    glEnable(GL_DEPTH_TEST);

    // Assign textures
    createTextureAttachment(app, wShader.rtRefraction, wShader.fboRefraction);
    createDepthTextureAttachment(app, wShader.rtRefractDepth, wShader.fboRefraction);

    glDrawBuffers(2, attachments);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void createTextureAttachment(App* app, GLuint& id, unsigned int fboBuffer)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
}

void createDepthTextureAttachment(App* app, GLuint& id, unsigned int fboBuffer)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, app->displaySize.x, app->displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id, 0);
}

void RenderInGBuffer(App* app)
{
    glEnable(GL_DEPTH_TEST);
    //Skybox
    glBindFramebuffer(GL_FRAMEBUFFER, app->gBuffer);
    // Draw function
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    RenderSkybox(app, app->cam);

    Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
    texturedMeshProgram.Bind();

    RenderScene(app, *app->cam, texturedMeshProgram);

    RenderWater(app);
}

void RenderSkybox(App* app, Camera*  cam)
{
    glDisable(GL_DEPTH_TEST);
    Program& skyProgram = app->programs[app->skyboxProgramId];
    skyProgram.Bind();
    skyProgram.glUniformMatrix4("projection", cam->projMatrix);
    skyProgram.glUniformMatrix4("view", cam->viewMatrix);

    app->enviroment.BindEnviroment(0);

    Model& cubeModel = app->models[app->cubeModel];
    cubeModel.Render(app, skyProgram);

    glEnable(GL_DEPTH_TEST);
}

void RenderWater(App* app)
{
    Program& waterShader = app->programs[app->waterProgramIdx];
    waterShader.Bind();

    waterShader.glUniformMatrix4("projectionMatrix", app->cam->projMatrix);
    waterShader.glUniformMatrix4("worldViewMatrix", app->cam->viewMatrix * app->waterEffect.waterPlaneEntity->worldMatrix);
    waterShader.glUniformVec2("viewportSize", app->displaySize);
    waterShader.glUniformMatrix4("modelViewMatrix", app->cam->viewMatrix); // TODO later model Matrix?
    waterShader.glUniformMatrix4("viewMatrixInv", glm::inverse(app->cam->viewMatrix));
    waterShader.glUniformMatrix4 ("projectionMatrixInv", glm::inverse(app->cam->projMatrix));

    waterShader.glUniformInt("reflectionMap", 0);
    waterShader.glUniformInt("refractionMap", 1);
    waterShader.glUniformInt("reflectionDepth", 2);
    waterShader.glUniformInt("refractionDepth", 3);
    waterShader.glUniformInt("normalMap", 4);
    waterShader.glUniformInt("dudvMap", 5);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->waterEffect.rtReflection);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, app->waterEffect.rtRefraction);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, app->waterEffect.rtReflectDepth);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, app->waterEffect.rtRefractDepth);

    app->textures[app->normalWaterIdx].Bind(4);
    app->textures[app->dudvWaterIdx].Bind(5);

    Model& plane = app->models[app->waterEffect.waterPlaneEntity->modelIndex];
    plane.Render(app, waterShader);
}

void LightingPass(App* app)
{
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Deferred Shading ======
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Draw in the screen ===
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Program& lightingProgram = app->programs[app->lightingPassProgram];
    glUseProgram(lightingProgram.handle);

    glBindBuffer(GL_UNIFORM_BUFFER, app->deferredbuffer.handle);
    app->deferredbuffer.head = 0;
    app->deferredbuffer.data = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);

    // GlobalParams
    app->globalParamsOffset = app->deferredbuffer.head;

    PushVec3(app->deferredbuffer, app->cam->position);
    PushUInt(app->deferredbuffer, app->lights.size());

    for (u32 i = 0; i < app->lights.size(); ++i)
    {
        AlignHead(app->deferredbuffer, sizeof(vec4));

        Light& light = app->lights[i];
        PushUInt(app->deferredbuffer, light.type);
        PushVec3(app->deferredbuffer, normalize(light.color));
        PushVec3(app->deferredbuffer, normalize(light.direction));
        PushVec3(app->deferredbuffer, light.position);

    }

    app->globalParamsSize = app->deferredbuffer.head - app->globalParamsOffset;

    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->deferredbuffer.handle, app->globalParamsOffset, app->globalParamsSize);

    lightingProgram.glUniformInt("renderMode", app->renderMode);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->gDiffuse);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, app->gNormals);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, app->gDepth);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, app->gPosition);

    Model& modelQuad = app->models[app->quadModel];
    modelQuad.Render(app, lightingProgram);

    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
