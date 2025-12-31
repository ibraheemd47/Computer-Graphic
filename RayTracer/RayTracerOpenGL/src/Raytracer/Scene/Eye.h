#pragma once

#include <glm/glm.hpp>
class Eye
{
public:
    glm::vec3 position;      
    int modeFlag;            
    
    glm::vec3 Vto;           
    glm::vec3 Vup;           
    float screenDist;        
    float screenWidth;       
    float screenHeight;      

    Eye(const glm::vec3 &position = glm::vec3(0.0f), int modeFlag = 0);

    void print() const;
};