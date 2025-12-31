#pragma once

#include <glm/glm.hpp>
class Ambient
{
public:
    glm::vec3 intensity; 

    Ambient(const glm::vec3 &intensity);

    void print() const;
};
