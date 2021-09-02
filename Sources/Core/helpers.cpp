#include "Core/helpers.h"

void helpers::scale2dVec(glm::vec2 &vecToScale, const ft scaleToLength)
{
	ft h = hypot(vecToScale.x, vecToScale.y);
	vecToScale.x = scaleToLength * (vecToScale.x / h);
	vecToScale.y = scaleToLength * (vecToScale.y / h);
}

glm::vec2 helpers::randomDirVector(CustomRand &customRand, const ft len)
{
	glm::vec2 unscaledRandomVec = glm::vec2(customRand.NextFloat(-1, 1), customRand.NextFloat(-1, 1));
	scale2dVec(unscaledRandomVec, len);
	return unscaledRandomVec;
}

std::vector<glm::vec2> helpers::transformStdVector(const std::vector<glm::vec3> &stdVec, const glm::mat4 &model)
{
	const ui vecSize = stdVec.size();
	std::vector<glm::vec2> returnVec;
	returnVec.reserve(vecSize);
	for (ui i = 0; i < vecSize; ++i)
		returnVec.push_back(glm::vec2(model * glm::vec4(stdVec[i], 1)));
	return returnVec;
}

std::vector<glm::vec2> helpers::transformStdVector(const UntexturedMeshParams &params, const glm::mat4 &model)
{
	std::vector<glm::vec2> returnVec;
	returnVec.reserve(params.noVertices);
	for (ui i = 0; i < params.noVertices; ++i)
		returnVec.push_back(glm::vec2(model * glm::vec4(params.positions[i], 1)));
	return returnVec;
}

helpers::BoundingBox::BoundingBox(const UntexturedMeshParams &params, const glm::mat4 &transform)
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

bool helpers::BoundingBox::IsThereAnIntersection(const glm::vec2 &vec) const
{
	return vec.x >= minCoords.x  &&
	vec.x <= maxCoords.x &&
	vec.y >= minCoords.y  &&
	vec.y <= maxCoords.y;
}

bool helpers::BoundingBox::IsThereAnIntersection(const std::vector<glm::vec2> &vecs) const
{
	return std::any_of(vecs.begin(), vecs.end(), [this](auto &vec){ return IsThereAnIntersection(vec); });
}

glm::vec2 helpers::mouseCoordsTransformed(const glm::mat4 &transform)
{
	int x, y;
	const ft halfScreenWidth = static_cast<ft>(SCREEN_WIDTH) / 2.0f;
	const ft halfScreenHeight = static_cast<ft>(SCREEN_HEIGHT) / 2.0f;
	SDL_GetMouseState(&x, &y);
	ft squishedX = (static_cast<ft>(x) - halfScreenWidth) / halfScreenWidth;
	ft squishedY = (static_cast<ft>(y) - halfScreenHeight ) /halfScreenHeight;
	glm::vec2 transformesdPos = glm::vec2(transform * glm::vec4(squishedX, squishedY, 0, 1));
	return transformesdPos;
}

bool helpers::IsLBMPressed()
{
	int x, y;
	Uint32 buttons = SDL_GetMouseState(&x, &y);
    return buttons & SDL_BUTTON_LMASK;
}

void helpers::render(Shader &shader, Mesh &mesh)
{
	shader.Bind();
	mesh.Draw();
}

void helpers::render(Shader &shader, Mesh &mesh, const glm::mat4 &transform, const glm::mat4 &projection)
{
	shader.Bind();
	shader.Update(transform, projection);
	mesh.Draw();
}

void helpers::render(Shader &shader, UntexturedInstancedMesh &mesh, const glm::mat4 *instanceTransforms, ui noInstances, const glm::mat4 &transform, const glm::mat4 &projection)
{
	shader.Bind();
	shader.Update(transform, projection);
	mesh.SetInstanceCount(noInstances);
	mesh.Update(instanceTransforms, mesh.InstancedBufferPosition());
	mesh.Draw();
}

ui helpers::squishedIntToScreenWidth(ui minValue, ui maxValue, ui value)
{
	ui relativeValue = value - minValue;
	ui relativeMaxValue = maxValue - minValue;
	ft valueToMaxFraction = static_cast<ft>(relativeValue) / static_cast<ft>(relativeMaxValue);
	return SCREEN_WIDTH * valueToMaxFraction;
}