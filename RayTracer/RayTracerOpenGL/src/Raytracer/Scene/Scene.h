#pragma once


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/dual_quaternion.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <vector>
#include <stdexcept>
#include "Raytracer/LightSources/LightSource.h"
#include <Raytracer/Core/Intersection.h>
#include "Raytracer/Core/Ray.h"
#include <Raytracer/Objects/Object.h>
#include <Raytracer/Scene/Ambient.h>
#include <Raytracer/Scene/Eye.h>

class Scene
{
public:
    Eye eye;
    Ambient ambient;
    std::vector<LightSource *> lights;
    std::vector<Object *> objects;

    Scene(const Eye &eye, const Ambient &ambient);

    ~Scene();

    void addLight(LightSource *light);
    void addObject(Object *obj);
    int getNumLights();
    Intersection GetHit(Ray &ray);

    LightSource *getLight(int num);

    void print() const;
};
