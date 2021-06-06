#include "engine.h"
GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128] = "\n";
    if(shaderName != "")
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
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(vertexShaderDefine),
        (GLint)programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(fragmentShaderDefine),
        (GLint)programSource.len
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
        attribute.location = glGetAttribLocation(program.handle, attributeName);

        program.vertexInputLayout.attributes.push_back(attribute);
    }
    app->programs.push_back(program);

    return app->programs.size() - 1;
}

void Program::Bind()
{
    glUseProgram(handle);
}

void Program::glUniformInt(const char* name, int value)
{
    GLuint location = glGetUniformLocation(handle, name);
    glUniform1i(location, value);
}

void Program::glUniformFloat(const char* name, float value)
{
    GLuint location = glGetUniformLocation(handle, name);
    glUniform1f(location, value);
}

void Program::glUniformMatrix4(const char* name, glm::mat4 mat4)
{
    GLuint MatrixID = glGetUniformLocation(handle, name);
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(mat4));
}

void Program::glUniformVec3(const char* name, glm::vec3 vec)
{
    GLuint location = glGetUniformLocation(handle, name);
    glUniform3f(location,vec.x, vec.y, vec.z);
}

void Program::glUniformVec4(const char* name, glm::vec4 vec)
{
    GLuint location = glGetUniformLocation(handle, name);
    glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
}

void Program::glUniformVec2(const char* name, glm::vec2 vec)
{
    GLuint location = glGetUniformLocation(handle, name);
    glUniform2f(location, vec.x, vec.y);
}


