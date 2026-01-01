#include "Phong.h"
#include "Raytracer/Scene/Scene.h"
#include "Raytracer/LightSources/DirectionalLight.h"
#include "Raytracer/LightSources/SpotLight.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#define MAX_LEVEL 4

glm::vec3 Phong::calcColor(Scene &scene, Ray &ray, int level) {                         
    Intersection hit = scene.GetHit(ray);

    int status = hit.ObjectStatus;  

    glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

    if (!hit.hitObject) {    
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }

    glm::vec3 normal = glm::normalize(hit.normal);

    if (status == 0) { 
        color = calcEmissionColor(scene) + calcAmbientColor(scene, hit);

        for (int i = 0; i < scene.getNumLights(); i++) {
            LightSource *light = scene.getLight(i);

            if (!occluded(scene, hit, light, normal)) {
                glm::vec3 specularColor = calcSpecularColor(scene, hit, light, ray, normal); 
                color += (calcDiffuseColor(scene, hit, light, normal) + specularColor) * light->intensity;
            }
        }
    }

    else if (status == 1) {
        if (level >= MAX_LEVEL) {
            return glm::vec3(0.0f);  
        }

        glm::vec3 normal = hit.normal;  
        Ray outRay = ConstructOutRay(ray, normal, hit.point);
        outRay.objectId = hit.objectId; 

        glm::vec3 reflectedColor = calcColor(scene, outRay, level + 1);
        color += reflectedColor;
    }

    else if (status == 2) {
        if (level >= MAX_LEVEL) return glm::vec3(0.0f);

    const float EPS = 1e-3f;

    Ray enterRay = calcTransparencyRay(ray, glm::normalize(hit.normal), hit.point);

    Ray insideRay(hit.point + EPS * enterRay.direction, enterRay.direction);

    Intersection exitHit = scene.GetHit(insideRay);
    if (!exitHit.hitObject || exitHit.objectId != hit.objectId)
        return glm::vec3(0.0f);

    glm::vec3 exitPoint = exitHit.point;

    
    Ray outsideRay = calcTransparencyRay(insideRay, glm::normalize(exitHit.normal), exitPoint);

    Ray finalRay(exitPoint + EPS * outsideRay.direction, outsideRay.direction);

    return calcColor(scene, finalRay, level + 1);
    }

    return color;
}

glm:: vec3 Phong ::checkerboardColor(glm::vec3 rgbColor, glm :: vec3 hitPoint) {
    float scaleParameter = 0.5f;
    float checkerboard = 0;
    if (hitPoint.x < 0) {
        checkerboard += floor((0.5 - hitPoint.x) / scaleParameter);
    }
    else {
        checkerboard += floor(hitPoint.x / scaleParameter);
    }
    if (hitPoint.y < 0) {
        checkerboard += floor((0.5 - hitPoint.y) / scaleParameter);
    }
    else {
        checkerboard += floor(hitPoint.y / scaleParameter);
    }
    checkerboard = (checkerboard * 0.5) - int(checkerboard * 0.5);
    checkerboard *= 2;
    if (checkerboard > 0.5) {
        return 0.5f * rgbColor;
    }
return rgbColor;
}



glm::vec3 Phong::calcAmbientColor(Scene &scene, Intersection &hit)
{
    return scene.ambient.intensity *  hit.getColor();
}

glm::vec3 Phong::calcEmissionColor(Scene &scene)
{
    return glm::vec3(0.0f, 0.0f, 0.0f); 
}



glm::vec3 Phong::calcDiffuseColor(Scene& scene, Intersection &hit, LightSource* light, const glm::vec3 &normal) {
    glm::vec3 Li; 
    glm::vec3 diffuseColor;
    glm::vec3 N = normal;   
    glm::vec3 KD = hit.getColor();  


    if (light->isDirectional()) {
        DirectionalLight* dirLight = static_cast<DirectionalLight*>(light); 

        Li = -glm::normalize(dirLight->direction); 
        float dotLN = glm::dot(N,Li);
        dotLN = glm::max(0.0f, dotLN);  

        diffuseColor = KD * dotLN ; 
    }
    else if (light->isSpotlight()) {
        Spotlight* spotLight = static_cast<Spotlight*>(light);  

        Li = glm::normalize(spotLight->position - hit.point);
        float dotLN = glm::dot(N,Li);
        dotLN = glm::max(0.0f, dotLN);  

        diffuseColor = KD * dotLN ; 
    }

    return diffuseColor ;
}

 glm::vec3 Phong::calcSpecularColor(Scene &scene, Intersection &hit, LightSource *light, Ray& ray, const glm::vec3 &normal) {
    glm::vec3 Ks = glm::vec3(0.7, 0.7, 0.7);  
    float n = hit.material.shininess;          
    glm::vec3 v = glm::normalize(ray.origin - hit.point);  
    glm::vec3 Ri;                             

    glm::vec3 N = normal;

    glm::vec3 lightDir;
    if (light->isDirectional()) {
        lightDir = -glm::normalize(dynamic_cast<DirectionalLight*>(light)->direction);
    } else if (light->isSpotlight()) {
        lightDir = glm::normalize(dynamic_cast<Spotlight*>(light)->position - hit.point);
    }

    Ri = glm::reflect(-lightDir, N);

    float specDot = glm::max( 0.0f, glm::dot(v, Ri)); 
    float specularFactor = glm::pow(specDot, n); 
    glm::vec3 specularColor = Ks * specularFactor ; 

    return specularColor;
}

bool Phong::occluded(Scene &scene, Intersection &hit, LightSource *light, const glm::vec3 &normal) {
    glm::vec3 lightDir;

    float maxDist = std::numeric_limits<float>::infinity();
    if (light->isDirectional()) {
        lightDir = -glm::normalize(dynamic_cast<DirectionalLight *>(light)->direction);
    } else if (light->isSpotlight()) {
        Spotlight *spotLight = dynamic_cast<Spotlight *>(light);
        lightDir = glm::normalize(spotLight->position - hit.point);

        if (!spotLight->isWithinCutoff(hit.point)) {
            return true;
        }
        maxDist = glm::length(spotLight->position - hit.point);
    }

    const float EPSILON = 1e-4f; 
    glm::vec3 shadowRayOrigin = hit.point + normal * EPSILON;
    Ray shadowRay(shadowRayOrigin, lightDir);
    shadowRay.objectId = hit.objectId; 

    Intersection shadowHit = scene.GetHit(shadowRay);

    bool blocked = false;
    if (shadowHit.hitObject) {
        if (shadowHit.t > EPSILON && shadowHit.t < maxDist) {
            blocked = true;
        }
    }

    const bool DEBUG_SHADOW = false;
    if (DEBUG_SHADOW) {
        std::cout << "Shadow check: obj " << hit.objectId << " lightDir(" << lightDir.x << "," << lightDir.y << "," << lightDir.z
                  << ") blocked=" << blocked << " t=" << shadowHit.t << " maxDist=" << maxDist << " hitId=" << shadowHit.objectId << "\n";
    }

    return blocked;
}




Ray Phong::ConstructOutRay(Ray &ray, glm::vec3 normal, glm::vec3 hitPoint) {
    glm::vec3 normalizedDirection = glm::normalize(ray.direction);
    glm::vec3 normalizedNormal = glm::normalize(normal);

    glm::vec3 reflectedDirection = normalizedDirection - 2.0f * glm::dot(normalizedDirection, normalizedNormal) * normalizedNormal;

    const float EPSILON = 1e-4f;
    glm::vec3 offsetOrigin = hitPoint + EPSILON * normalizedNormal;

    return Ray(offsetOrigin, glm::normalize(reflectedDirection));
}



Ray Phong::calcTransparencyRay(const Ray &ray, const glm::vec3 &normal, const glm::vec3 &hitPosition) {
    const float n_air = 1.0f;
    const float n_glass = 1.5f;   
    const float EPS = 1e-4f;

    glm::vec3 I = glm::normalize(ray.direction);
    glm::vec3 N = glm::normalize(normal);

    float cosi = glm::clamp(glm::dot(I, N), -1.0f, 1.0f);

    float etai = n_air;
    float etat = n_glass;

    if (cosi > 0.0f) {
        std::swap(etai, etat);
        N = -N;
    }

    cosi = std::abs(cosi);

    float eta = etai / etat;
    float k = 1.0f - eta * eta * (1.0f - cosi * cosi);

    if (k < 0.0f) {
        glm::vec3 R = glm::normalize(glm::reflect(I, N));
        return Ray(hitPosition + EPS * R, R);
    }

    glm::vec3 T = glm::normalize(eta * I + (eta * cosi - std::sqrt(k)) * N);

    return Ray(hitPosition + EPS * T, T);
}