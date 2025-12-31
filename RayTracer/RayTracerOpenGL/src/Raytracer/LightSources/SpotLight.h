#pragma once
#include "LightSource.h"

class Spotlight : public LightSource
{
public:
    glm::vec3 position; 
    float cutoff;       

    Spotlight(const glm::vec3 &intensity, const glm::vec3 &direction, float cutoff, const glm::vec3 &position);
    bool isDirectional() const override { return false; }
    bool isSpotlight() const override { return true; }

    glm::vec3   getDirectionToPoint(const glm::vec3 &point) ;
    bool isWithinCutoff(const glm::vec3 &point) ;


    void print() const override;
};

