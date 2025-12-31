#include "Scene.h"
#include "Raytracer/Phong.h"
#include "Raytracer/Objects/Plane.h"
#include "Raytracer/Objects/Sphere.h"
#include <limits>
#include <glm/glm.hpp>

Scene::Scene(const Eye &eye, const Ambient &ambient)
    : eye(eye), ambient(ambient) {}

void Scene::addLight(LightSource *light)
{
    lights.push_back(light);
}

void Scene::addObject(Object *obj)
{
    objects.push_back(obj);
}

void Scene::print() const
{
    std::cout << "Scene Details:" << std::endl;
    eye.print();
    ambient.print();
    for (const auto &light : lights)
        light->print();
    for (const auto &obj : objects)
        obj->print();
}

int Scene ::getNumLights()
{
    return lights.size();
}

Scene::~Scene()
{
    for (auto light : lights)
    {
        delete light; 
    }
    for (auto obj : objects)
    {
        delete obj; 
    }
}

Intersection Scene::GetHit(Ray &ray) {
   Intersection closestIntersection;
    closestIntersection.t = std::numeric_limits<float>::infinity(); 
    closestIntersection.hitObject = false;

    for (Object *obj : objects) {
            if (ray.objectId != -1 && obj->ObjectId == ray.objectId) {
                continue;
            }

            float t = 0.0f; 
            if (obj->Intersect(ray, t)) { 
                if (t < closestIntersection.t) { 
                    closestIntersection.t = t;
                    closestIntersection.point = ray.pointAtParameter(t);

                    if (obj->isPlane()) {
                        Plane *plane = dynamic_cast<Plane *>(obj);

                        glm::vec3 normal(plane->coefficients.x, plane->coefficients.y, plane->coefficients.z);
                        if (glm::dot(normal, ray.direction) > 0.0f) {
                            normal = -normal;
                        }
                        closestIntersection.normal = glm::normalize(normal);
                        closestIntersection.material = plane->material; 
                        closestIntersection.ObjectType = "Plane";  
                        closestIntersection.objectId = obj->ObjectId;   

                    }
                    else if (obj->isSphere()) {
                        Sphere *sphere = dynamic_cast<Sphere *>(obj);
                        closestIntersection.normal = glm::normalize(closestIntersection.point - sphere->center);
                        closestIntersection.material = sphere->material; 
                        closestIntersection.ObjectType = "Sphere"; 
                        closestIntersection.objectId = obj->ObjectId;   

                    }

                    closestIntersection.hitObject = true; 
                    closestIntersection.ObjectStatus = obj->status; 
                }
            }
    }
        
    return closestIntersection; 
}


LightSource *Scene::getLight(int num)
{
    return lights.at(num);
}
