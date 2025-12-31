#define GLM_ENABLE_EXPERIMENTAL
#include "Eye.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

Eye::Eye(const glm::vec3 &position, int modeFlag)
    : position(position), modeFlag(modeFlag),
      Vto(0.0f, 0.0f, -1.0f),    
      Vup(0.0f, 1.0f, 0.0f),     
      screenDist(1.0f),           
      screenWidth(2.0f),          
      screenHeight(2.0f)          
{}

void Eye::print() const
{
    std::cout << "Eye - Position: " << glm::to_string(position)
              << ", ModeFlag: " << modeFlag 
              << ", Vto: " << glm::to_string(Vto)
              << ", Vup: " << glm::to_string(Vup)
              << ", ScreenDist: " << screenDist
              << ", ScreenWidth: " << screenWidth
              << ", ScreenHeight: " << screenHeight << std::endl;
}