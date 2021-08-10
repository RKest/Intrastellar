#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>
#include <GL/glew.h>
#include "Core/transform.h"
#include "Core/camera.h"

#include "_config.h"

class Shader
{
public:
    Shader(const std::string& filePath, const std::initializer_list<const std::string>& paramNames, ui noParams, const std::initializer_list<const std::string>& uniformNames, ui noUniforms, bool hasGeometryShader = false);
    
    void Bind();
    void Update(Transform &transform, const Camera &camera);
    void Update(Transform &transform);
    
    void SetMat4(const std::string &name, const glm::mat4 &arg);
    void SetFloat(const std::string &name, float arg);
    void SetVec3(const std::string &name, const glm::vec3 &arg);
    void SetInt(const std::string &name, unsigned int arg);

    static void SetMat4(Shader *shaders, unsigned int noShaders, const std::string &name, const glm::mat4 &arg);
    static void SetVec3(Shader *shaders, unsigned int noShaders, const std::string &name, const glm::vec3&arg);
    static void SetFloat(Shader *shaders, unsigned int noShaders, const std::string &name, float arg);

    virtual ~Shader();

protected:
private:
    enum Uniforms
    {
        TRANSFORM_U,
        PROJECTION_U,

        NUM_UNIFORMS
    };

    void SetMat4(Uniforms uId, const glm::mat4 &arg)
    {
        glUniformMatrix4fv(uniforms[uId], 1, GL_FALSE, &arg[0][0]);
    }

    void SetVec3(Uniforms uId, const glm::vec3 &arg)
    {
        glUniform3fv(uniforms[uId], 1, &arg[0]);
    }

    GLuint program;
    //0 == Vertex Shader; 1 == Fragment Shader
    std::vector<GLuint> shaders;
    GLuint uniforms[NUM_UNIFORMS];

    glm::mat4 blankTransform = glm::translate(glm::vec3(0,0,0));
};

#endif // Shader_H
