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


GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const char* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);

    GLint attributeCount;
    glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    for (GLint i = 0; i < attributeCount; i++)
    {
        VertexShaderAttribute attribute;
        char attributeName[255];
        GLsizei attributeNameLenght;
        GLint attributeSize;
        GLenum attributeType;

        glGetActiveAttrib(program.handle, i,
            ARRAY_COUNT(attributeName),
            &attributeNameLenght,
            &attributeSize,
            &attributeType,
            attributeName);

        attribute.name = attributeName;
        attribute.componentCount = (u8)attributeSize;
        attribute.location =  glGetAttribLocation(program.handle, attributeName);

        program.vertexInputLayout.attributes.push_back(attribute);
    }
    app->programs.push_back(program);

    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
        case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
        default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
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

void glUniformMatrix4(u32 programID, const char* name, glm::mat4 mat4)
{
    GLuint MatrixID = glGetUniformLocation(programID, name);

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(mat4));
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


    // Create Camera
    app->cam = new Camera(60.0f, 0.1f, 1000.0f, (float)(app->displaySize.x/app->displaySize.y));

    // Load Program
    app->texturedMeshProgramIdx = LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
    Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
    app->textureMeshProgram_uTexture = glGetUniformLocation(texturedMeshProgram.handle, "uTexture");

    LoadModel(app, "Patrick/Patrick.obj");

    app->mode = Mode_TexturedQuad;

    //Entities =====
    app->entities.push_back(Entity(vec3(0.0, 0.0, 0.0)));
    app->entities.push_back(Entity(vec3(2.0, 0.0, 0.0)));
    app->entities.push_back(Entity(vec3(-2.0, 0.0, 0.0)));


    // Create Light
    Light light = Light(LightType_Directional, vec3(1.0,1,1), vec3(0, -1,0 ), vec3(10, 10, 10));
    app->lights.push_back(light);


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

    // Todo apply changes to camera when properties modified

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
    if (app->input.IsKeyPressed(K_A))
        app->cam->position += app->cameraSpeed * app->deltaTime * app->cam->right;
    else if (app->input.IsKeyPressed(K_D))
        app->cam->position -= app->cameraSpeed * app->deltaTime * app->cam->right;
    else if (app->input.IsKeyPressed(K_W))
        app->cam->position.y += app->cameraSpeed * app->deltaTime;
    else if (app->input.IsKeyPressed(K_S))
        app->cam->position.y -= app->cameraSpeed * app->deltaTime;
    else if (app->input.IsKeyPressed(K_X))
        app->cam->position += app->cameraSpeed * app->deltaTime * app->cam->front;
    else if (app->input.IsKeyPressed(K_Z))
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
                // Draw function
                glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glEnable(GL_DEPTH_TEST);

                glViewport(0, 0, app->displaySize.x, app->displaySize.y);

                Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
                glUseProgram(texturedMeshProgram.handle);
          
                glBindBuffer(GL_UNIFORM_BUFFER, app->cbuffer.handle);
                app->cbuffer.head = 0;
                app->cbuffer.data = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);


                // GlobalParams
                app->globalParamsOffset = app->cbuffer.head;

                PushVec3(app->cbuffer, app->cam->position);
                PushUInt(app->cbuffer, app->lights.size());

                for (u32 i = 0; i < app->lights.size(); ++i)
                {
                    AlignHead(app->cbuffer, sizeof(vec4));

                    Light& light = app->lights[i];
                    PushUInt(app->cbuffer, light.type);
                    PushVec3(app->cbuffer, normalize(light.color));
                    PushVec3(app->cbuffer, normalize(light.direction));
                    PushVec3(app->cbuffer, light.position);

                }

                app->globalParamsSize = app->cbuffer.head - app->globalParamsOffset;

                glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->cbuffer.handle, app->globalParamsOffset, app->globalParamsSize);

                // Local Params
                for (int i = 0; i < app->entities.size(); ++i)
                {
                    AlignHead(app->cbuffer, app->uniformBlockAligment); // TODO set the 0 value to an uniformBlockAligment 
                    Entity& entity = app->entities[i];
                    glm::mat4    world = entity.worldMatrix;
                    glm::mat4    worldViewProjection = app->cam->projViewMatrix * entity.worldMatrix;

                    entity.localParamsOffset = app->cbuffer.head;
                    PushMat4(app->cbuffer, world);
                    PushMat4(app->cbuffer, worldViewProjection);
                    entity.localParamsSize = app->cbuffer.head - entity.localParamsOffset;

                    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->cbuffer.handle, entity.localParamsOffset, entity.localParamsSize);

                    Model& model = app->models[entity.modelIndex];
                    Mesh& mesh = app->meshes[model.meshIdx];

                    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                    {
                        GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                        glBindVertexArray(vao);

                        u32 submeshMaterialIdx = model.materialIdx[i];
                        Material& submeshMaterial = app->materials[submeshMaterialIdx];

                        glUniform1i(app->textureMeshProgram_uTexture, 0);
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);

                        Submesh& submesh = mesh.submeshes[i];
                        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                    }
                }

                glUnmapBuffer(GL_UNIFORM_BUFFER);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
            break;

        default:;
    }
}