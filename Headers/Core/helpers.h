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
    glm::vec2 mouseCoordsTransformed(const glm::mat4 &transform);
    bool IsLBMPressed();
    ft det(const glm::vec2 &vec1, const glm::vec2 &vec2);
    ui squishedIntToScreenWidth(ui minValue, ui maxValue, ui value);
    ft angleBetweenPoints(const glm::mat4 &from, const glm::mat4 &to, const glm:vec2 &up = glm::vec2(1.0f, 0.0f));

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

    template <typename T>
    std::vector<T> flattenVec(const std::vector<std::vector<T>>& v) 
    {
        std::size_t total_size = 0;
        for (const auto& sub : v)
           total_size += sub.size();
         std::vector<T> result;
        result.reserve(total_size);
        for (const auto& sub : v)
            result.insert(result.end(), sub.begin(), sub.end());
         return result;
    }

    template <typename T>
    std::size_t twoDVecSize(const std::vector<std::vector<T>>& v)
    {
        std::size_t total_size = 0;
        for (const auto& sub : v)
           total_size += sub.size();
         return total_size;
    }
    
    template<container_type Cont, typename Call>
    void forEach(Cont &cont, Call f)
    {
        std::for_each(begin(cont), end(cont), f);
    }

    struct Core
    {
        Camera &camera;
        Text &text;
        Timer &timer;
        PlayerStats &stats;
    };

}