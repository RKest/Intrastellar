#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glew.h>
#include "Ext/obj_loader.h"

#include "_config.h"

#ifndef STATIC_DUMMY_MAT
#define STATIC_DUMMY_MAT
static glm::mat4 dummyMat = glm::translate(glm::vec3(0,4,0));
#endif

struct UntexturedMeshParams
{
    const glm::vec3 *positions;
    const ui *indices;
    const ui noVertices;
    const ui noIndices;
};

class VAB
{
public:
    VAB(GLuint *vertexArrayBuffers);
    template <typename T>
    void Init(const T *bufferDataPtr, ui noBufferElements, ui bufferPosition, GLenum type = GL_FLOAT);
    void Index(const ui *indicesPtr, ui noIndices, ui bufferPos);
    void Instance(GLuint *vertexArrayBuffers, ui startingBufferPos, ui size = 4, ui maxNoInstances = 10);

private:
    GLuint *vertexArrayBuffers;
    GLuint vertexArrayObject;
};

class Mesh
{
public:
    Mesh();
    virtual void Draw();

protected:
    GLuint vertexArrayObject;
    void SetDrawCount(ui arg) { drawCount = arg; };
    ui drawCount;
};

class InstancedMesh : public Mesh
{
public:
    //Templating didn't work
    void Update(const glm::mat4 *matrices, GLuint bufferAtPos);
    void Update(const glm::mat3 *matrices, GLuint bufferAtPos);
    void Draw() override;

    inline void SetInstanceCount(ui arg) { instanceCount = arg; }

protected:
    size_t instanceOffset = 0;
private:
    ui instanceCount = 0;
    GLuint *vertexArrayBuffers;
    ui instancedBufferPosition;
};

class UntexturedMesh : public Mesh
{
public:
    UntexturedMesh(const UntexturedMeshParams &params);

private:
    VAB vab;
    enum
    {
        POSITION_VB,
        INDEX_VB,

        NO_BUFFERS
    };

    GLuint vertexArrayBuffers[NO_BUFFERS];
};

class UntexturedInstancedMesh : public InstancedMesh
{
public:
    UntexturedInstancedMesh(const UntexturedMeshParams &params, ui maxNoInstances = 10, ui transformMatSize = 4);
    inline GLuint InstancedBufferPosition() { return vertexArrayBuffers[INSTANCE_TRANSFORM_VB]; }

private:
    VAB vab;
    enum
    {
        POSITION_VB,
        INDEX_VB,
        INSTANCE_TRANSFORM_VB,

        NO_BUFFERS
    };

    GLuint vertexArrayBuffers[NO_BUFFERS];
};

#endif //MESH_H