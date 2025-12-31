#define GLM_ENABLE_EXPERIMENTAL
#include "Sphere.h"
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
Sphere::Sphere(const Material &material, int status, const glm::vec3 &center, float radius)
{
    this->material = material;
    this->status = status;
    this->center = center;
    this->radius = radius;
}

Sphere::Sphere(int status, const glm::vec3 &center, float radius)
{
    this->status = status;
    this->center = center;
    this->radius = radius;
}

void Sphere::print() const
{
    std::cout << "Sphere - Center: " << glm::to_string(center)
              << ", Radius: " << radius << std::endl;
    material.print();
}

bool Sphere::Intersect(Ray &ray, float &t)
{
    glm::vec3 oc = ray.origin - center;

    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0)
    {
        return false;
    }

    float sqrtDiscriminant = sqrt(discriminant);
    float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
    float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

    if (t1 >= 0 && t2 >= 0)
    {
        t = fmin(t1, t2); 
        return true;
    }
    if (t1 >= 0)
    {
        t = t1; 
        return true;
    }
    if (t2 >= 0)
    {
        t = t2; 
        return true;
    }

    return false;
}
