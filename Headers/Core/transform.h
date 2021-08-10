#ifndef MY_TRANSFORM_H
#define MY_TRANSFORM_H

#include "_config.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

class Transform
{
public:
    Transform(const glm::vec3 &pos = glm::vec3(), const glm::vec3 &rot = glm::vec3(0, 0, 1), const glm::vec3 &scale = glm::vec3(1.0f, 1.0f, 1.0f)) : pos(pos),
                                                                                                                                                     rot(rot),
                                                                                                                                                     scale(scale) {}
    glm::mat4 Model();

    inline glm::vec3 &Pos() { return pos; }
    inline glm::vec3 &Rot() { return rot; }
    inline glm::vec3 &Scale() { return scale; }
    inline ft &RotAngle() { return rotAngle; }

    inline void SetPos(const glm::vec3 &arg) { pos = arg; }
    inline void SetRot(const glm::vec3 &arg) { rot = arg; }
    inline void SetScale(const glm::vec3 &arg) { scale = arg; }
    inline void SetRotAngle(const ft arg) { rotAngle = arg; }

protected:
private:
    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 scale;
    ft rotAngle = 0.0f;
    glm::mat4 QuaternionRotate(const glm::vec3 &axis, ft angle);
};

#endif // TRANSFORM_H