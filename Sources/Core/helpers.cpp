#include "Core/helpers.h"

glm::vec2 helpers::scale2dVec(const glm::vec2 &vecToScale, const ft scaleToLength)
{
	const ft h = hypot(vecToScale.x, vecToScale.y);
	const glm::vec2 returnVec{scaleToLength * (vecToScale.x / h), scaleToLength * (vecToScale.y / h)};
	return returnVec;
}

glm::vec2 helpers::randomDirVector(CustomRand &customRand, const ft len)
{
	glm::vec2 unscaledRandomVec = glm::vec2(customRand.NextFloat(-1, 1), customRand.NextFloat(-1, 1));
	glm::vec2 scaledRandomVec = scale2dVec(unscaledRandomVec, len);
	return scaledRandomVec;
}

std::vector<glm::vec2> helpers::transformStdVector(const std::vector<glm::vec3> &stdVec, const glm::mat4 &model)
{
	const size_t vecSize = stdVec.size();
	std::vector<glm::vec2> returnVec;
	returnVec.reserve(vecSize);
	for (size_t i = 0; i < vecSize; ++i)
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

void helpers::transformMatVec(std::vector<glm::mat4> &vec, const glm::mat4 &model)
{
	std::for_each(vec.begin(), vec.end(), [&model](auto &mat){ mat *= model; });
}

void helpers::transformMatVec(std::vector<glm::mat4> &vec, const ft yTransformVal)
{
	const glm::mat4 model = glm::translate(glm::vec3(0, yTransformVal, 0));
	std::for_each(vec.begin(), vec.end(), [&model](auto &mat){ mat *= model; });
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

ui helpers::squishedIntToScreenWidth(ui minValue, ui maxValue, ui value)
{
	ui relativeValue = value - minValue;
	ui relativeMaxValue = maxValue - minValue;
	ft valueToMaxFraction = static_cast<ft>(relativeValue) / static_cast<ft>(relativeMaxValue);
	return decl_cast(relativeValue, decl_cast(valueToMaxFraction, SCREEN_WIDTH) * valueToMaxFraction);
}

ft helpers::det(const glm::vec2 &vec1, const glm::vec2 &vec2)
{
    return vec1.x * vec2.y - vec1.y * vec2.x;
}

ft angleBetweenPoints(const glm::mat4 &from, const glm::mat4 &to, const glm:vec2 &up = glm::vec2(1.0f, 0.0f))
{
	const glm::vec2 toPos{to * glm::vec4(0,0,0,1)};
	const glm::vec2 fromPos = glm::normalize(glm::vec2(glm::inverse(from) * glm::vec4(toPos, 0, 1)));
	const ft angle = -glm::atan(glm::dot(up, fromPos), helpers::det(up, toPos));
}

const glm::mat4 helpers::transformTowards(const glm::mat4 &from, const glm::mat4 &to, const ft byHowMutch)
{
	const glm::vec2 toPos	{to * glm::vec4(0,0,0,1)};
	const glm::vec2 fromPos {from * glm::vec4(0,0,0,1)};
	const glm::vec2 vecBetween{toPos - fromPos};
	const glm::vec2 scaledVecBetween = helpers::scale2dVec(vecBetween, byHowMutch);
	const glm::mat4 localTransform = glm::translate(glm::vec3(scaledVecBetween , 0));
	return localTransform;
}