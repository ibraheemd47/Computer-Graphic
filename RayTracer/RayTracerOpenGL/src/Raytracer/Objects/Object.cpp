#include "Object.h"

bool Object::isSphere() const
{
    return false;
}

bool Object::isPlane() const
{
    return false;
}

void Object ::setMaterial(Material &material)
{
    this->material = material;
}
