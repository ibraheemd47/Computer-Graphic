#ifndef SCENE_READER_H
#define SCENE_READER_H
#include <string>
#include <vector>
#include "Scene/Scene.h"

class SceneReader {
public:
Eye* eye;                  
Ambient* ambient;          
std::vector<Object*> objects;
std::vector<LightSource*> lights;

SceneReader() : eye(nullptr), ambient(nullptr) {}

Scene* readScene(const std::string& filename);

static  Ray  ConstructRayThroughPixel(float x, float y, int width, int height, Scene & scene);


};


#endif 
