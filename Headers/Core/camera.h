#pragma once

#include "_config.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

class Camera
{
public:
    Camera() = delete;
    inline static void Construct(const glm::vec3 &pos, ft fov, ft aspect, ft zNear, ft zFar)
    {
        perspective = glm::perspective(fov, aspect, zNear, zFar);
        position = pos;
        forward = glm::vec3(0, 0, 1);
        up = glm::vec3(0, 1, 0);
        Recalc();
    }

    inline static glm::vec3 &Pos() { return position; }
    inline static glm::vec3 &Fwd() { return forward; }
    inline static glm::vec3 &Up()  { return up; }

    inline static void Recalc()
    {
        projection = perspective * glm::lookAt(position, position + forward, up); 
    }

    inline static glm::mat4 &ViewProjection() const
    {
        return projection;
    }

protected:
private:
    inline static glm::mat4 projection;
    inline static glm::mat4 perspective;
    inline static glm::vec3 position;
    inline static glm::vec3 forward;
    inline static glm::vec3 up;
};