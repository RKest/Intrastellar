#pragma once
#include "_config.h"
#include "Core/mesh.h"

#include <array>

struct BoundingBox
{
	bool IsThereAnIntersection(const std::vector<glm::mat4>&, ui&) const;
	bool IsThereAnIntersection(const glm::mat4&) const;
	virtual bool IsThereAnIntersection(const glm::vec2&) const = 0;
};

struct ReqBoundingBox : public BoundingBox
{
	ReqBoundingBox() = default;
	ReqBoundingBox(const UntexturedMeshParams &params, const glm::mat4 &transform = _blankTransform);
	ReqBoundingBox(const TexturedMeshParams &params, const glm::mat4 &transform = _blankTransform);
	glm::vec2 minDimentions;
	glm::vec2 maxDimentions;
	glm::vec2 minCoords;
	glm::vec2 maxCoords;
	void UpdateCoords(const glm::mat4&);
	bool IsThereAnIntersection(const std::vector<glm::mat4>&, ui&) const;
	bool IsThereAnIntersection(const glm::vec2&) const override;
	bool IsThereAnIntersection(const std::vector<glm::vec2>&) const;
};

struct TriBoundingBox : public BoundingBox
{
	TriBoundingBox() = default;
	TriBoundingBox(const UntexturedMeshParams &params, const glm::mat4 &transform = _blankTransform);
	std::array<glm::vec2, 3> triangleCoords;
	std::array<glm::vec2, 3> trianglePoss;
	void UpdateCoords(const glm::mat4&);
	bool IsThereAnIntersection(const std::vector<glm::mat4>&, ui&) const;
	bool IsThereAnIntersection(const glm::vec2&) const override;
private:
	//Courtasy of https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
	inline ft sign(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3) const 
	{
 	   return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
	}
};