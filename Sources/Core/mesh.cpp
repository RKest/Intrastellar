#include "Core/mesh.h"

VAB::VAB(GLuint *vertexArrayBuffers)
    : vertexArrayBuffers(vertexArrayBuffers)
{
}

template <typename T>
void VAB::Init(const T *bufferDataPtr, ui noBufferElements, ui bufferPos, GLenum mode, GLenum type)
{
    glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffers[bufferPos]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bufferDataPtr[0]) * noBufferElements, bufferDataPtr, mode);

    glEnableVertexAttribArray(bufferPos);
    glVertexAttribPointer(bufferPos, sizeof(bufferDataPtr[0]) / sizeof(type), type, GL_FALSE, 0, 0);
}

void VAB::Index(const ui *indicesPtr, ui noIndices, ui bufferPos)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexArrayBuffers[bufferPos]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesPtr[0]) * noIndices, indicesPtr, GL_STATIC_DRAW);
}

void VAB::Instance(GLuint *vertexArrayBuffers, ui startingBufferPos, ui size, ui maxNoInstances)
{
    const GLsizei instanceSize = sizeof(GLfloat) * size * size;
    glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffers[startingBufferPos]);
    glBufferData(GL_ARRAY_BUFFER, maxNoInstances * instanceSize, NULL, GL_DYNAMIC_DRAW);

    for (ui i = 0; i < size; ++i)
    {
        glEnableVertexAttribArray(startingBufferPos + i);
        glVertexAttribPointer(startingBufferPos + i, 4, GL_FLOAT, GL_FALSE, instanceSize, (void *)(sizeof(GLfloat) * i * size));
        glVertexAttribDivisor(startingBufferPos + i, 1);
    }
}

Mesh::Mesh()
{
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
}

void Mesh::Draw()
{
    glBindVertexArray(vertexArrayObject);
    glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void InstancedMesh::Draw()
{
    glBindVertexArray(vertexArrayObject);
    glDrawElementsInstanced(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, 0, instanceCount);
    glBindVertexArray(0);
}

//Templating didn't work 
void InstancedMesh::Update(const glm::mat4 *matrices, GLuint bufferAtPos)
{
    glBindVertexArray(vertexArrayObject);
    const size_t sz = sizeof(matrices[0]) * instanceCount;
    glBindBuffer(GL_ARRAY_BUFFER, bufferAtPos);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sz, matrices);
    glBindVertexArray(0);
}

void InstancedMesh::Update(const glm::mat3 *matrices, GLuint bufferAtPos)
{
    glBindVertexArray(vertexArrayObject);
    const size_t sz = sizeof(matrices[0]) * instanceCount;
    glBindBuffer(GL_ARRAY_BUFFER, bufferAtPos);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sz, matrices);
    glBindVertexArray(0);
}

UntexturedMesh::UntexturedMesh(const UntexturedMeshParams &params)
    : vab(vertexArrayBuffers)
{
    SetDrawCount(params.noIndices);
    glGenBuffers(NO_BUFFERS, vertexArrayBuffers);

    vab.Init(params.positions, params.noVertices, POSITION_VB);
    vab.Index(params.indices, params.noIndices, INDEX_VB);

    glBindVertexArray(0);
}

UntexturedDynamicMesh::UntexturedDynamicMesh(const UntexturedMeshParams &params)
    : vab(vertexArrayBuffers)
{
    SetDrawCount(params.noIndices);
    glGenBuffers(NO_BUFFERS, vertexArrayBuffers);

    vab.Init(params.positions, params.noVertices, POSITION_VB, GL_DYNAMIC_DRAW);
    vab.Index(params.indices, params.noIndices, INDEX_VB);

    glBindVertexArray(0);
}

void UntexturedDynamicMesh::Update(const glm::vec3 *positions, const ui noPositions)
{
    glBindVertexArray(vertexArrayObject);
    const size_t sz = sizeof(positions[0]) * noPositions;
    glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffers[POSITION_VB]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sz, positions);
    glBindVertexArray(0);
}

UntexturedInstancedMesh::UntexturedInstancedMesh(const UntexturedMeshParams &params, ui maxNoInstances, ui transformMatSize)
    : vab(vertexArrayBuffers)
{    
    SetDrawCount(params.noIndices);
    glGenBuffers(NO_BUFFERS, vertexArrayBuffers);

    vab.Init(params.positions, params.noVertices, POSITION_VB);
    vab.Index(params.indices, params.noIndices, INDEX_VB);
    vab.Instance(vertexArrayBuffers, INSTANCE_TRANSFORM_VB, transformMatSize, maxNoInstances);

    glBindVertexArray(0);
}
