#pragma once

#include <glm/glm.hpp>

class Material
{
public:
    glm::vec3 color; 
    float shininess; 
    Material(const glm::vec3 &color = glm::vec3(1.0f), float shininess = 0.0f);

    void print() const;
};
