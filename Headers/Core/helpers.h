#ifndef HELPERS_H
#define HELPERS_H

#include "_config.h"
#include "Core/random.h"
#include "Core/mesh.h"
#include "Core/display.h"
#include "Core/camera.h"
#include "Core/shader.h"

#include <vector>

namespace helpers
{
    void scale2dVec(glm::vec2 &vecToScale, const ft scaleToLength);
    glm::vec2 randomDirVector(CustomRand &customRand, const ft len);
    std::vector<glm::vec2> transformStdVector(const std::vector<glm::vec3> &stdVec, const glm::mat4 &model);
    std::vector<glm::vec2> transformStdVector(const UntexturedMeshParams &params, const glm::mat4 &model);
    struct BoundingBox
    {
        BoundingBox() = default;
        BoundingBox(const UntexturedMeshParams &params, const glm::mat4 &transform = _blankTransform);
        glm::vec2 minDimentions;
        glm::vec2 maxDimentions;
        glm::vec2 minCoords;
        glm::vec2 maxCoords;
        bool IsThereAnIntersection(const glm::vec2&) const;
        bool IsThereAnIntersection(const std::vector<glm::vec2>&) const;
    };
    glm::vec2 mouseCoordsTransformed(const glm::mat4 &transform);
    bool IsLBMPressed();
    void render(Shader &shader, Mesh &mesh);
    void render(Shader &shader, Mesh &mesh, const glm::mat4 &transfor, const glm::mat4 &projection);
    void render(Shader &shader, UntexturedInstancedMesh &mesh, const glm::mat4 *instanceTransforms, ui noInstances, const glm::mat4 &transform, const glm::mat4 &projection);
    ui squishedIntToScreenWidth(ui minValue, ui maxValue, ui value);
    template<typename T>
    void pushToCappedVector(std::vector<T> &cappedVec, const T &el, ui &oldestElIndex const ui cap)
    {
        if (cappedVec.size() == cap)
		{
			cappedVec[oldestElIndex] = el;
			oldestElIndex = (oldestElIndex + 1) % cap;
		}
		else
			cappedVec.push_back(el);
    }
}

#endif