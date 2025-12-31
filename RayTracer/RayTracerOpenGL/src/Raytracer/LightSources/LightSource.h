#pragma once

#include <glm/glm.hpp>

class LightSource
{
public:
    glm::vec3 direction;
    glm::vec3 intensity; 

    LightSource(const glm::vec3 &intensity, const glm::vec3 &direction);

    virtual ~LightSource() = default; 

    virtual void print() const;

    virtual bool isDirectional() const;
    virtual bool isSpotlight() const;
};
