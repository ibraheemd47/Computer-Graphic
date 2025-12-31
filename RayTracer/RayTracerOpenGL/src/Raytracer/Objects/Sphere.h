#pragma once

#include "Raytracer/Objects/Object.h"
class Sphere : public Object
{
public:
    glm::vec3 center; 
    float radius;     
    Sphere(const Material &material, int status, const glm::vec3 &center, float radius);
    Sphere(int status, const glm::vec3 &center, float radius);

    bool Intersect(Ray &ray, float &t) override;
    void print() const override;

    bool isSphere() const override { return true; }
    bool isPlane() const override { return false; }
};