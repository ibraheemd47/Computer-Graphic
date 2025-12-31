#pragma once

#include "LightSource.h"

class DirectionalLight : public LightSource
{
public:
    DirectionalLight(const glm::vec3 &intensity, const glm::vec3 &direction);
    bool isDirectional() const override { return true; }
    bool isSpotlight() const override { return false; }

    void print() const override;
};