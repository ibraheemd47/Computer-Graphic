#include "Intersection.h"
#include <limits>
#include "Raytracer/Phong.h"
Intersection::Intersection() : point(glm::vec3(0.0f)), normal(glm::vec3(0.0f)), t(0.0f), hitObject(false) {}

Intersection::Intersection(const glm::vec3 &point, const glm::vec3 &normal, float t, bool hitObject)
    : point(point), normal(normal), t(t), hitObject(hitObject) {}

bool Intersection::isValid() const
{
    return hitObject;
}

glm::vec3 Intersection ::getColor()
{
    return (ObjectType.compare("Plane") == 0) ? Phong::checkerboardColor(material.color, point) : material.color;
}