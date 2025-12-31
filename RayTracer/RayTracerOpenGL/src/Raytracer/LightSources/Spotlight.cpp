#include "Spotlight.h"

#include <glm/glm.hpp>
#include <iostream>


Spotlight::Spotlight(const glm::vec3 &intensity, const glm::vec3 &direction, float cutoff, const glm::vec3 &position)
    : LightSource(intensity, direction), cutoff(cutoff), position(position) {};

void Spotlight::print() const
{
    std::cout << "Spotlight, Intensity:(" << intensity.x << ", " << intensity.y << ", " << intensity.z << ")"
              << ", Direction: " << direction.x << ", " << direction.y << ", " << direction.z << ")"
              << ", Position: " << position.x << ", " << position.y << ", " << position.z << ")"
              << ", Cutoff: " << cutoff << std::endl;
}


glm::vec3 Spotlight :: getDirectionToPoint(const glm::vec3 &point)  {
        return glm::normalize(point - position);
    }



 bool Spotlight :: isWithinCutoff(const glm::vec3 &point)  {
        glm::vec3 directionToPoint = getDirectionToPoint(point);
        float cosTheta = glm::dot(glm::normalize(direction), directionToPoint);
        return cosTheta >= cutoff;
}

