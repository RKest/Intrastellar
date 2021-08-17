#ifndef HELPERS_H
#define HELPERS_H

#include "_config.h"
#include "Core/random.h"

namespace helpers 
{
    void scale2dVec(glm::vec2 &vecToScale, const ft scaleToLength);
    glm::vec2 randomDirVector(CustomRand &customRand, const ft len);
}

#endif