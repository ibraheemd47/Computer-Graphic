#include "LightSource.h"
#include <iostream>

LightSource::LightSource(const glm::vec3 &intensity, const glm::vec3 &direction)
    : intensity(intensity), direction(direction) {}


void LightSource::print() const
{
}

bool LightSource::isDirectional() const
{
    return false;
}

bool LightSource::isSpotlight() const
{
    return false;
}
