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
