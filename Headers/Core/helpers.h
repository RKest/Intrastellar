#pragma once

#include "_config.h"
#include "Core/random.h"
#include "Core/mesh.h"
#include "Core/display.h"
#include "Core/camera.h"
#include "Core/shader.h"
#include "Core/text.h"
#include "Core/timer.h"

#include <vector>
#include <algorithm>
#include <iterator>
#include <string>


namespace helpers
{
    [[nodiscard]]glm::vec2 scale2dVec(const glm::vec2 &vecToScale, const ft scaleToLength);
    glm::vec2 randomDirVector(CustomRand &customRand, const ft len);
    std::vector<glm::vec2> transformStdVector(const std::vector<glm::vec3> &stdVec, const glm::mat4 &model);
    std::vector<glm::vec2> transformStdVector(const UntexturedMeshParams &params, const glm::mat4 &model);
    void transformMatVec(std::vector<glm::mat4> &vec, const glm::mat4 &model);
    void transformMatVec(std::vector<glm::mat4> &vec, const ft yTransformVal);
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
        bool IsThereAnIntersection(const std::vector<glm::mat4> &transforms, ui &intersectionIndex) const;
    };
    glm::vec2 mouseCoordsTransformed(const glm::mat4 &transform);
    bool IsLBMPressed();
    ft det(const glm::vec2 &vec1, const glm::vec2 &vec2);

    template<typename ...T>
    void render(Shader &shader, Mesh &mesh, std::pair<std::string, T> const&... params)
    {
        shader.Bind();
        if constexpr (sizeof...(params) > 0)
            shader.SetUnis(params...);
        mesh.Draw();
    }

    template<typename ...T>
    void render(Shader &shader, Mesh &mesh, const glm::mat4 &transform, const glm::mat4 &projection, std::pair<std::string, T> const&... params)
    {
        shader.Bind();
        if constexpr (sizeof...(params) > 0)
            shader.SetUnis(params...);
        shader.Update(transform, projection);
        mesh.Draw();
    }
    
    template<typename ...T>
    void render(Shader &shader, UntexturedInstancedMesh &mesh, const glm::mat4 *instanceTransforms, size_t noInstances, const glm::mat4 &transform, 
        const glm::mat4 &projection, std::pair<std::string, T> const&... params)
    {
        shader.Bind();
        if constexpr (sizeof...(params) > 0)
            shader.SetUnis(params...);
        shader.Update(transform, projection);
        mesh.SetInstanceCount(static_cast<ui>(noInstances));
        mesh.Update(instanceTransforms, mesh.InstancedBufferPosition());
        mesh.Draw();
    }
    ui squishedIntToScreenWidth(ui minValue, ui maxValue, ui value);
    template<typename T>
    constexpr void pushToCappedVector(std::vector<T> &cappedVec, const T &el, ui &oldestElIndex, const ui cap)
    {
        if (cappedVec.size() == cap)
		{
			cappedVec[oldestElIndex] = el;
			oldestElIndex = (oldestElIndex + 1) % cap;
		}
		else
			cappedVec.push_back(el);
    }

    template<typename T>
    const std::vector<T> &flattenVector(const std::vector<std::vector<T>> &stdVec)
    {
        return std::accumulate(stdVec.begin(), stdVec.end(), decltype(stdVec)::value_type(),
            [](auto& dest, auto& src) {
                dest.insert(dest.end(), src.begin(), src.end());
                return dest;
            });
    }

    struct Core
    {
        Camera &camera;
        Text &text;
        Timer &timer;
        PlayerStats &stats;
    };

    // template<container_type Cont, typename Call>
    // void forEach(Cont &cont, Call f)
    // {
    //     std::for_each(begin(cont), end(cont), f);
    // }
}