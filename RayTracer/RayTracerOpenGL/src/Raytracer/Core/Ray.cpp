#include "Ray.h"
#include <glm/glm.hpp>
#include <limits>
Ray ::Ray(const glm::vec3 &origin, const glm::vec3 &direction)
    : origin(origin), direction(glm::normalize(direction)) {
        objectId = -1;
    } 

glm::vec3 Ray ::pointAtParameter(float t) const
{
    return origin + t * direction;
}