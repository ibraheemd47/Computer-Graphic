#define GLM_ENABLE_EXPERIMENTAL
#include "Plane.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
Plane::Plane(const Material &material, int status, const glm::vec4 &coefficients)
{
    this->material = material;
    this->status = status;
    this->coefficients = coefficients;
}

Plane::Plane(int status, const glm::vec4 &coefficients)
{
    this->status = status;
    this->coefficients = coefficients;
}

bool Plane::Intersect(Ray &ray, float &t)
{
    float a = coefficients.x;
    float b = coefficients.y;
    float c = coefficients.z;
    float d = coefficients.w;

    glm::vec3 normal(a, b, c);
    float denominator = glm::dot(normal, ray.direction);

    if (glm::abs(denominator) < 1e-6)
    {
        return false; 
    }

    float numerator = -(glm::dot(normal, ray.origin) + d);

    t = numerator / denominator;

    return t >= 0;
}

void Plane::print() const
{
    std::cout << "Plane - Coefficients: " << glm::to_string(coefficients) << std::endl;
    material.print();
}
