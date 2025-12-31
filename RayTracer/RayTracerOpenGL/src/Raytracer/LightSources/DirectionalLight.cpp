#include "DirectionalLight.h"
#include <iostream>

DirectionalLight::DirectionalLight(const glm::vec3 &intensity, const glm::vec3 &direction)
    : LightSource(intensity, direction) {}

void DirectionalLight::print() const
{
    std::cout << "Directional Light, Intensity: ("
              << intensity.x << ", " << intensity.y << ", " << intensity.z << ")"
              << ", Direction: ("
              << direction.x << ", " << direction.y << ", " << direction.z << ")"
              << std::endl;
}