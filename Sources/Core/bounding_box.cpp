#include "Core/bounding_box.h"

bool BoundingBox::IsThereAnIntersection(const std::vector<glm::mat4> &transforms, ui &collisionIndex) const 
{
	const auto b = transforms.cbegin();
    const auto e = transforms.cend();
    const auto itr = std::find_if(b, e, [this](auto &transform) { return IsThereAnIntersection(transform); });
    if (itr == e)
        return false;
    collisionIndex = decl_cast(collisionIndex, std::distance(b, itr));
    return true;
}
bool BoundingBox::IsThereAnIntersection(const glm::mat4 *transforms, const size_t sz) const
{
	for(size_t i = 0; i < sz; ++i)
		if(IsThereAnIntersection(transforms[i]))
			return true;
	return false;
}
bool BoundingBox::IsThereAnIntersection(const glm::mat4 &transform) const
{
	const glm::vec2 pos{transform * glm::vec4(0,0,0,1)};
	return IsThereAnIntersection(pos);
}
bool BoundingBox::IsThereAnIntersection(const glm::mat4 *transforms, const size_t sz, ui &collisionIndex) const
{
	for(size_t i = 0; i < sz; ++i)
		if(IsThereAnIntersection(transforms[i]))
		{
			collisionIndex = i;
			return true;
		}
	return false;
}
ReqBoundingBox::ReqBoundingBox(const UntexturedMeshParams &params, const glm::mat4 &transform)
{
	ft minX = 100.0f, maxX = -100.0f, minY = 100.0f, maxY = -100.0f;
	for (ui i = 0; i < params.noVertices; ++i)
	{
		if (params.positions[i][0] > maxX) maxX = params.positions[i][0];
		if (params.positions[i][0] < minX) minX = params.positions[i][0];
		if (params.positions[i][1] > maxY) maxY = params.positions[i][1];
		if (params.positions[i][1] < minY) minY = params.positions[i][1];
	}
	minDimentions = glm::vec2(minX, minY);
	maxDimentions = glm::vec2(maxX, maxY);
	minCoords = glm::vec2(transform * glm::vec4(minDimentions, 0, 1));
	maxCoords = glm::vec2(transform * glm::vec4(maxDimentions, 0, 1));
}
ReqBoundingBox::ReqBoundingBox(const TexturedMeshParams &params, const glm::mat4 &transform)
{
	ft minX = 100.0f, maxX = -100.0f, minY = 100.0f, maxY = -100.0f;
	for (ui i = 0; i < params.noVertices; ++i)
	{
		if (params.positions[i][0] > maxX) maxX = params.positions[i][0];
		if (params.positions[i][0] < minX) minX = params.positions[i][0];
		if (params.positions[i][1] > maxY) maxY = params.positions[i][1];
		if (params.positions[i][1] < minY) minY = params.positions[i][1];
	}
	minDimentions = glm::vec2(minX, minY);
	maxDimentions = glm::vec2(maxX, maxY);
	minCoords = glm::vec2(transform * glm::vec4(minDimentions, 0, 1));
	maxCoords = glm::vec2(transform * glm::vec4(maxDimentions, 0, 1));
}
void ReqBoundingBox::UpdateCoords(const glm::mat4 &transform)
{
	minCoords = glm::vec2(transform * glm::vec4(minDimentions, 0, 1));
	maxCoords = glm::vec2(transform * glm::vec4(maxDimentions, 0, 1));
}
bool ReqBoundingBox::IsThereAnIntersection(const std::vector<glm::mat4> &transforms, ui &collisionIndex) const
{
	return BoundingBox::IsThereAnIntersection(transforms, collisionIndex);
}
bool ReqBoundingBox::IsThereAnIntersection(const glm::mat4 *transforms, const size_t sz) const
{
	return BoundingBox::IsThereAnIntersection(transforms, sz);
}
bool ReqBoundingBox::IsThereAnIntersection(const glm::mat4 *transforms, const size_t sz, ui &collisionIndex) const
{
	return BoundingBox::IsThereAnIntersection(transforms, sz, collisionIndex);
}
bool ReqBoundingBox::IsThereAnIntersection(const glm::vec2 &vec) const 
{
	return vec.x >= minCoords.x  &&
	vec.x <= maxCoords.x &&
	vec.y >= minCoords.y  &&
	vec.y <= maxCoords.y;
}
bool ReqBoundingBox::IsThereAnIntersection(const std::vector<glm::vec2> &vecs) const
{
	return std::any_of(vecs.begin(), vecs.end(), [this](auto &vec){ return IsThereAnIntersection(vec); });
}
bool ReqBoundingBox::IsThereAnIntersection(const glm::vec2 &originPos, const ft shotAngle) const
{
	const glm::vec2 farAwayPos = helpers::vecDistanceAway(originPos, 99.0f, shotAngle);
	return 	doLinesIntersect(originPos, farAwayPos, minCoords, glm::vec2(minCoords.x, maxCoords.y)) ||
			doLinesIntersect(originPos, farAwayPos, minCoords, glm::vec2(maxCoords.x, minCoords.y)) ||
			doLinesIntersect(originPos, farAwayPos, maxCoords, glm::vec2(minCoords.x, maxCoords.y)) ||
			doLinesIntersect(originPos, farAwayPos, maxCoords, glm::vec2(maxCoords.x, minCoords.y));
}
TriBoundingBox::TriBoundingBox(const UntexturedMeshParams &params, const glm::mat4 &transform)
{
	for(ui i = 0; i < params.noVertices; ++i)
	{
		triangleCoords[i] = glm::vec2(transform * glm::vec4(params.positions[i], 1));
		trianglePoss[i]   = glm::vec2(params.positions[i]);
	}
}
void TriBoundingBox::UpdateCoords(const glm::mat4 &transform)
{
	for(std::size_t i = 0; i < trianglePoss.size(); ++i)
		triangleCoords[i] = glm::vec2(transform * glm::vec4(trianglePoss[i], 0, 1));
}
bool TriBoundingBox::IsThereAnIntersection(const std::vector<glm::mat4> &transforms, ui &collisionIndex) const
{
	return BoundingBox::IsThereAnIntersection(transforms, collisionIndex);
}
bool TriBoundingBox::IsThereAnIntersection(const glm::mat4 *transforms, const size_t sz) const
{
	return BoundingBox::IsThereAnIntersection(transforms, sz);
}
bool TriBoundingBox::IsThereAnIntersection(const glm::mat4 *transforms, const size_t sz, ui &collisionIndex) const
{
	return BoundingBox::IsThereAnIntersection(transforms, sz, collisionIndex);
}
bool TriBoundingBox::IsThereAnIntersection(const glm::vec2 &pos) const 
{
	const ft d1 = sign(pos, triangleCoords[0], triangleCoords[1]);
	const ft d2 = sign(pos, triangleCoords[1], triangleCoords[2]);
	const ft d3 = sign(pos, triangleCoords[2], triangleCoords[0]);
	const bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	const bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
	return !(has_neg && has_pos);
}
bool TriBoundingBox::IsThereAnIntersection(const glm::vec2 &originPos, const ft shotAngle) const
{
	const glm::vec2 farAwayPos = helpers::vecDistanceAway(originPos, 99.0f, shotAngle);
	return 	doLinesIntersect(originPos, farAwayPos, triangleCoords[0], triangleCoords[1]) ||
			doLinesIntersect(originPos, farAwayPos, triangleCoords[1], triangleCoords[2]) ||
			doLinesIntersect(originPos, farAwayPos, triangleCoords[2], triangleCoords[0]);
}
ft TriBoundingBox::sign(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3) const
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}