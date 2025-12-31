#define GLM_ENABLE_EXPERIMENTAL
#include "Raytracer/Materials/Material.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

Material::Material(const glm::vec3 &color, float shininess)
    : color(color), shininess(shininess) {}

void Material::print() const
{
    std::cout << "Material - Color: " << glm::to_string(color)
              << ", Shininess: " << shininess << std::endl;
}
