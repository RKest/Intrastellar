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

ft helpers::matDistance(const glm::mat4 &mat, const glm::vec2 &vec)
{
	return glm::distance(glm::vec2(mat * glm::vec4(0,0,0,1)), vec);
}
void helpers::transformMatVec(std::vector<glm::mat4> &vec, const ft yTransformVal)
{
	const glm::mat4 model = glm::translate(glm::vec3(0, yTransformVal, 0));
	std::for_each(vec.begin(), vec.end(), [&model](auto &mat){ mat *= model; });
}

glm::vec2 helpers::mouseCoordsTransformed(const glm::mat4 &transform, const ft transformScalingFactor)
{
	int x, y;
	const ft halfScreenWidth = static_cast<ft>(SCREEN_WIDTH) / 2.0f;
	const ft halfScreenHeight = static_cast<ft>(SCREEN_HEIGHT) / 2.0f;
	SDL_GetMouseState(&x, &y);
	ft squishedX = (static_cast<ft>(x) - halfScreenWidth) / halfScreenWidth;
	ft squishedY = (static_cast<ft>(y) - halfScreenHeight ) /halfScreenHeight;
	glm::vec2 transformesdPos = glm::vec2(transform * glm::vec4(squishedX, -squishedY, 0.0f, transformScalingFactor));
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

ft helpers::angleBetweenPoints(const glm::mat4 &from, const glm::mat4 &to, const glm::vec2 &up)
{
	const ft orientedAngle = glm::orientedAngle(up, glm::normalize(glm::vec2(glm::inverse(from) * to * glm::vec4(0,0,0,1))));
	const ft angle = orientedAngle < 0 ? TAU + orientedAngle : orientedAngle;
	return angle;
}

glm::mat4 helpers::transformTowards(const glm::mat4 &from, const glm::mat4 &to, const ft byHowMutch)
{
	const glm::vec2 toPos	{to * glm::vec4(0,0,0,1)};
	const glm::vec2 fromPos {from * glm::vec4(0,0,0,1)};
	const glm::vec2 vecBetween{toPos - fromPos};
	const glm::vec2 scaledVecBetween = helpers::scale2dVec(vecBetween, byHowMutch);
	const glm::mat4 localTransform = glm::translate(glm::vec3(scaledVecBetween , 0));
	return localTransform;
}

ft helpers::angleDiff(const ft a, const ft b)
{
	const ft delta = std::abs(a - b);
	if(delta > PI)
		return TAU - delta;
	else
		return delta;
}
ft helpers::rotTransformAngle(const glm::mat4 &matrix) {
	return glm::atan(matrix[0][1], matrix[0][0]);
}
glm::mat4 helpers::rotateTowardsClosest(const std::vector<glm::mat4> &rotateTowardModels, const glm::mat4 &rotateFromModel, const ft maxTurningRadius, const ft minDistanceToTurn)
{
	if(rotateTowardModels.empty())
		return _blankTransform;
		
	const glm::vec2 rotateFromPos{rotateFromModel * glm::vec4(0,0,0,1)};
	const auto closestModel = std::min_element(cbegin(rotateTowardModels), cend(rotateTowardModels), 
		[&rotateFromPos](auto &m1, auto &m2){ return helpers::matDistance(m1, rotateFromPos) < helpers::matDistance(m2, rotateFromPos); });

	const glm::vec2 closestPos{*closestModel * glm::vec4(0,0,0,1)};
	if(glm::distance(rotateFromPos, closestPos) > minDistanceToTurn)
		return _blankTransform; 
		
	return helpers::rotateTowards(rotateFromModel, *closestModel, maxTurningRadius);
}

glm::mat4 helpers::rotateTowards(const glm::mat4 &rotateTowardModel, const glm::mat4 &rotateFromModel, const ft maxTurningRadius)
{
	const ft angle = helpers::angleBetweenPoints(rotateFromModel, rotateTowardModel);
	if(angle < maxTurningRadius)
		return glm::rotate(angle, glm::vec3(0,0,1));
	if(TAU - angle < maxTurningRadius)
		return glm::rotate(-angle, glm::vec3(0,0,1));
	if(angle < PI)
		return glm::rotate(maxTurningRadius, glm::vec3(0,0,1));
	return glm::rotate(-maxTurningRadius, glm::vec3(0,0,1));
}