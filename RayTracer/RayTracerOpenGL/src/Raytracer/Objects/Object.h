#pragma once

#include <glm/glm.hpp>
#include "Raytracer/Materials/Material.h"
#include "Raytracer/Core/Ray.h"
#include "Raytracer/Core/Intersection.h"
class Object
{
public:
    int status; 
    Material material;
    int ObjectId ; 

    virtual bool isSphere() const;
    virtual bool isPlane() const;

    virtual ~Object() = default;
    void setMaterial(Material &material);
    virtual bool Intersect(Ray &ray, float &t) = 0; 
    virtual void print() const = 0;
};