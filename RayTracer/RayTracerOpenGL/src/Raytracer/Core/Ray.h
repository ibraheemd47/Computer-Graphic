#pragma once

#include <glm/glm.hpp>
class Ray
{
public:
    glm::vec3 origin;    
    glm::vec3 direction; 
    int objectId ;  

    Ray(const glm::vec3 &origin, const glm::vec3 &direction);

    glm::vec3 pointAtParameter(float t) const;
};