#pragma once

#include <glm/glm.hpp>
#include "Raytracer/Materials/Material.h"
#include "Raytracer/Core/Ray.h"
#include <string>
class Intersection
{
public:
    glm::vec3 point;  
    glm::vec3 normal; 
    Material material;
    float t;        
    bool hitObject; 
    std::string ObjectType;
    int ObjectStatus = 1 ; 
    int objectId ; 

    Intersection();

    Intersection(const glm::vec3 &point, const glm::vec3 &normal, float t, bool hitObject);

    glm::vec3 getColor();

    bool isValid() const;
};
