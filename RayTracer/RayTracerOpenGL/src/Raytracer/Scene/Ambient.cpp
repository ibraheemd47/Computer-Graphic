#define GLM_ENABLE_EXPERIMENTAL
#include "Ambient.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
Ambient::Ambient(const glm::vec3 &intensity) : intensity(intensity) {}

void Ambient::print() const
{
    std::cout << "Ambient - Intensity: " << glm::to_string(intensity) << std::endl;
}
