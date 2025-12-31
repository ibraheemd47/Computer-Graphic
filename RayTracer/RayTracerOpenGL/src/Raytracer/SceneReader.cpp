#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include "Raytracer/Scene/Scene.h"
#include "Raytracer/SceneReader.h"
#include "Raytracer/LightSources/DirectionalLight.h"
#include "Raytracer/LightSources/SpotLight.h"
#include "Raytracer/Objects/Sphere.h"
#include "Raytracer/Objects/Plane.h"
#include <bits/unique_ptr.h>
#include <bits/shared_ptr.h>

// CHANGE HERE: Debug flags
#define DEBUG_SCENE_PARSING 0
#define DEBUG_RAY_DIRECTIONS 0

Scene *SceneReader::readScene(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open scene file.");
    }

    Eye eye(glm::vec3(0.0f));
    Ambient ambient(glm::vec3(0.0f));
    std::vector<Object *> objects;
    std::vector<LightSource *> lights;

    std::string line;

    std::vector<Material> materials;                     
    std::vector<std::pair<glm::vec4, int>> objectDataList;  

    int sphereCount = 0;
    int planeCount = 0;
    int reflectiveCount = 0;
    int transparentCount = 0;
    int directionalCount = 0;
    int spotlightCount = 0;

    std::vector<glm::vec4> dirTokens;   
    std::vector<glm::vec4> spotPosTokens; 
    std::vector<glm::vec4> intTokens;    

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "e")
        { 
            glm::vec3 position;
            float screenDist;
            iss >> position.x >> position.y >> position.z >> screenDist;
            eye.position = position;
            eye.screenDist = screenDist;
        }
        else if (token == "u")
        { 
            glm::vec3 up;
            float screenHeight;
            iss >> up.x >> up.y >> up.z >> screenHeight;
            eye.Vup = up;
            eye.screenHeight = screenHeight;
        }
        else if (token == "f")
        { 
            glm::vec3 forward;
            float screenWidth;
            iss >> forward.x >> forward.y >> forward.z >> screenWidth;
            eye.Vto = forward;
            eye.screenWidth = screenWidth;
        }
        else if (token == "a")
        { 
            glm::vec3 intensity;
            float ignoreW;
            iss >> intensity.x >> intensity.y >> intensity.z >> ignoreW;
            ambient = Ambient(intensity);
        }
        else if (token == "c")
        { 
            glm::vec3 color;
            float shininess;
            iss >> color.x >> color.y >> color.z >> shininess;
            materials.emplace_back(color, shininess);
        }
        else if (token == "o" || token == "r" || token == "t")
        { 
            glm::vec4 objData;
            iss >> objData.x >> objData.y >> objData.z >> objData.w;
            int status = (token == "o") ? 0 : (token == "r") ? 1 : 2;
            objectDataList.push_back({objData, status});
        }
        else if (token == "p")
        { 
            glm::vec4 pos;
            iss >> pos.x >> pos.y >> pos.z >> pos.w;
            spotPosTokens.push_back(pos);
        }
        else if (token == "d")
        { 
            glm::vec4 d; 
            iss >> d.x >> d.y >> d.z >> d.w;
            dirTokens.push_back(d);
        }
        else if (token == "i")
        { 
            glm::vec4 c;
            iss >> c.x >> c.y >> c.z >> c.w;
            intTokens.push_back(c);
        }
    }

    
    for (size_t i = 0; i < objectDataList.size(); ++i)
    {
        glm::vec4 objData = objectDataList[i].first;
        int status = objectDataList[i].second;

        Material mat = (i < materials.size()) ? materials[i] : Material(glm::vec3(1.0f), 1.0f);

        if (objData.w > 0)
        {
            Sphere *sphere = new Sphere(status, glm::vec3(objData.x, objData.y, objData.z), objData.w);
            sphere->setMaterial(mat);
            objects.push_back(sphere);
            sphereCount++;
            if (status == 1) reflectiveCount++;
            if (status == 2) transparentCount++;
        }
        else
        {
            Plane *plane = new Plane(status, objData);
            plane->setMaterial(mat);
            objects.push_back(plane);
            planeCount++;
        }
    }

    size_t lightCount = dirTokens.size();

    for (size_t i = 0, spotIdx = 0; i < lightCount; ++i)
    {
        glm::vec3 direction = glm::normalize(glm::vec3(dirTokens[i]));
        float type = dirTokens[i].w;

        glm::vec3 intensity(1.0f);
        if (i < intTokens.size())
        {
            intensity = glm::vec3(intTokens[i]); 
        }
        else
        {
            std::cout << "WARNING: missing intensity for light index " << i << ", defaulting to (1,1,1)\n";
        }

        if (std::abs(type) < 1e-5f)
        {
            lights.push_back(new DirectionalLight(intensity, direction));
            directionalCount++;
        }
        else
        {
            
            if (spotIdx < spotPosTokens.size())
            {
                glm::vec4 pos = spotPosTokens[spotIdx++];
                lights.push_back(new Spotlight(intensity, direction, pos.w, glm::vec3(pos)));
                spotlightCount++;
            }
            else
            {
                std::cout << "WARNING: missing spotlight position for spotlight index " << spotIdx << "\n";
            }
        }
    }

    std::cout << "=== Scene Summary ===\n";
    std::cout << "Objects: " << objects.size() << " (spheres=" << sphereCount << ", planes=" << planeCount << ")\n";
    std::cout << "Materials: " << materials.size() << "\n";
    std::cout << "Lights: " << lights.size() << " (dir=" << directionalCount << ", spot=" << spotlightCount << ")\n";
    for (size_t i = 0, spotIdx = 0; i < dirTokens.size(); ++i) {
        glm::vec4 d = dirTokens[i];
        glm::vec3 inten = (i < intTokens.size()) ? glm::vec3(intTokens[i]) : glm::vec3(1.0f);
        std::cout << "  light[" << i << "] type=" << (std::abs(d.w) < 1e-5f ? "Directional" : "Spotlight")
                  << " dir=(" << d.x << "," << d.y << "," << d.z << ")"
                  << " inten=(" << inten.r << "," << inten.g << "," << inten.b << ")";
        if (std::abs(d.w - 1.0f) < 1e-5f) {
            if (spotIdx < spotPosTokens.size()) {
                glm::vec4 p = spotPosTokens[spotIdx];
                std::cout << " pos=(" << p.x << "," << p.y << "," << p.z << ") cutoff=" << p.w;
            }
            spotIdx++;
        }
        std::cout << "\n";
    }
    if (dirTokens.size() != intTokens.size()) {
        std::cout << "WARNING: d/i count mismatch (d=" << dirTokens.size() << ", i=" << intTokens.size() << ")\n";
    }
    if (static_cast<size_t>(spotlightCount) != spotPosTokens.size()) {
        std::cout << "WARNING: spotlight count != p count (spotlights=" << spotlightCount << ", p=" << spotPosTokens.size() << ")\n";
    }
    std::cout << "====================\n";

    Scene *scene = new Scene(eye, ambient);
    for (auto light : lights)
    {
        scene->addLight(light);
    }
    int id = 1;
    for (auto object : objects)
    {
        object->ObjectId = id;
        scene->addObject(object);
        id+=1;
    }

    return scene;
}

Ray SceneReader::ConstructRayThroughPixel(float x, float y, int width, int height, Scene &scene)
{
    glm::vec3 eyePos = scene.eye.position;
    glm::vec3 Vto = scene.eye.Vto;
    glm::vec3 Vup = scene.eye.Vup;
    float screenDist = scene.eye.screenDist;
    float screenWidth = scene.eye.screenWidth;
    float screenHeight = scene.eye.screenHeight;

    glm::vec3 forward = glm::normalize(Vto);

    
    glm::vec3 right = glm::normalize(glm::cross(forward, Vup));

    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    if (glm::length(right) < 1e-6f) {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    if (glm::length(up) < 1e-6f) {
        up = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    float sx = (x + 0.5f) / static_cast<float>(width);
    float sy = (y + 0.5f) / static_cast<float>(height);

    float px = (sx - 0.5f) * screenWidth;
    float py = (0.5f - sy) * screenHeight;

    glm::vec3 center = eyePos + forward * screenDist;
    glm::vec3 P = center + right * px + up * py;

    glm::vec3 direction = glm::normalize(P - eyePos);

    if (DEBUG_RAY_DIRECTIONS) {
        const bool isCorner = ((x == 0 || x == width - 1) && (y == 0 || y == height - 1));
        if (isCorner) {
            std::cout << "Ray dir at (" << x << "," << y << "): "
                      << direction.x << ", " << direction.y << ", " << direction.z << "\n";
        }
    }

    return Ray(eyePos, direction);
}