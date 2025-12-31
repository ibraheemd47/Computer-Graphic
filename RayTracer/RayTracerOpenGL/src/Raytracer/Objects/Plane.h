#pragma once

#include "Object.h"

class Plane : public Object
{
public:
    glm::vec4 coefficients; 

    Plane(const Material &material, int status, const glm::vec4 &coefficients);
    Plane(int status, const glm::vec4 &coefficients);

    bool Intersect(Ray &ray, float &t) override;
    void print() const override;

    bool isSphere() const override { return false; }
    bool isPlane() const override { return true; }
};