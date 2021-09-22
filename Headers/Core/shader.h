#ifndef SHADER_H
#define SHADER_H

#include "_config.h"

#include "Core/transform.h"
#include "Core/camera.h"

#include <GL/glew.h>
#include <string>
#include <vector>
#include <fstream>
#include <utility>

using matUni = std::pair<std::string, glm::mat4>;
using vecUni = std::pair<std::string, glm::vec3>;
using ftUni = std::pair<std::string, ft>;
using uiUni = std::pair<std::string, ui>;

static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string &errorMessage);
static std::string LoadShader(const std::string &fileName);
static GLuint CreateShader(const std::string &text, GLenum shaderType);

class Shader
{
public:
    template <size_t SZ = UNIFORMS.size()>
    Shader(const std::string &filePath, const std::array<const char *, SZ> &uniformNames = UNIFORMS, bool hasGeometryShader = false)
    {
        program = glCreateProgram();
        shaders.push_back(CreateShader(LoadShader(filePath + "/VS.glsl"), GL_VERTEX_SHADER));
        shaders.push_back(CreateShader(LoadShader(filePath + "/FS.glsl"), GL_FRAGMENT_SHADER));

        if (hasGeometryShader)
            shaders.push_back(CreateShader(LoadShader(filePath + "/GS.glsl"), GL_GEOMETRY_SHADER));

        for (ui i = 0; i < shaders.size(); ++i)
            glAttachShader(program, shaders[i]);

        glLinkProgram(program);
        CheckShaderError(program, GL_LINK_STATUS, true, "ERROR: Program linking failed");
        glValidateProgram(program);
        CheckShaderError(program, GL_VALIDATE_STATUS, true, "ERROR: Program is invalid");

        for (ui i = 0; i < SZ; ++i)
            uniforms[i] = glGetUniformLocation(program, uniformNames.begin()[i]);
    }

    void Bind() { glUseProgram(program); }
    void Update(const glm::mat4 &model, const glm::mat4 &cameraProjection) { SetUni(TRANSFORM_U, model); SetUni(PROJECTION_U, cameraProjection); }

    template<typename T, typename ...Ts>
    constexpr void SetUnis(const std::pair<std::string, T> first, Ts ...rest)
    {
        SetUni(first.first, first.second);
        if constexpr (sizeof...(rest) > 0)
            SetUnis(rest...);
    }

    void SetUni(const std::string &name, const glm::mat4 &arg) { glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &arg[0][0]); }
    void SetUni(const std::string &name, const glm::vec3 &arg) { glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &arg[0]); }
    void SetUni(const std::string &name, float arg)            { glUniform1f(glGetUniformLocation(program, name.c_str()), arg); }
    void SetUni(const std::string &name, unsigned int arg)     { glUniform1i(glGetUniformLocation(program, name.c_str()), arg); }

    ~Shader()
    {
        for (ui i = 0; i < shaders.size(); ++i)
        {
            glDetachShader(program, shaders[i]);
            glDeleteShader(shaders[i]);
        }
        glDeleteProgram(program);
    }

protected:
private:
    enum Uniforms
    {
        TRANSFORM_U,
        PROJECTION_U,

        NUM_UNIFORMS
    };

    void SetUni(Uniforms uId, const glm::mat4 &arg)
    {
        glUniformMatrix4fv(uniforms[uId], 1, GL_FALSE, &arg[0][0]);
    }

    void SetUni(Uniforms uId, const glm::vec3 &arg)
    {
        glUniform3fv(uniforms[uId], 1, &arg[0]);
    }

    GLuint program;
    //0 == Vertex Shader; 1 == Fragment Shader
    std::vector<GLuint> shaders;
    GLuint uniforms[NUM_UNIFORMS];
};

static std::string LoadShader(const std::string &fileName)
{
    std::ifstream file;
    file.open((fileName).c_str());

    std::string output;
    std::string line;

    if (file.is_open())
    {
        while (file.good())
        {
            getline(file, line);
            output.append(line + '\n');
        }
    }
    else
    {
        std::cerr << "Unable to open shader: " << fileName << std::endl;
    }

    return output;
}

static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string &errorMessage)
{
    GLint success = 0;
    GLchar error[1024] = {0};

    if (isProgram)
        glGetProgramiv(shader, flag, &success);
    else
        glGetShaderiv(shader, flag, &success);

    if (success == GL_FALSE)
    {
        if (isProgram)
            glGetProgramInfoLog(shader, sizeof(error), NULL, error);
        else
            glGetShaderInfoLog(shader, sizeof(error), NULL, error);

        std::cerr << errorMessage << ": '" << error << "'" << std::endl;
    }
}

static GLuint CreateShader(const std::string &text, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);

    if (shader == 0)
        std::cerr << "ERROR: Shader creation failed" << std::endl;

    const GLchar *shaderSourceStrings[1];
    GLint shaderSourceStringLengths[1];

    shaderSourceStrings[0] = text.c_str();
    shaderSourceStringLengths[0] = static_cast<GLuint>(text.length());

    glShaderSource(shader, 1, shaderSourceStrings, shaderSourceStringLengths);
    glCompileShader(shader);

    CheckShaderError(shader, GL_COMPILE_STATUS, false, "ERROR: Shader failed to compile");

    return shader;
}

#endif // Shader_H
