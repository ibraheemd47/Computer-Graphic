#version 300 es
precision highp float;
precision highp int;

struct Camera {
    vec3 pos;
    vec3 forward;
    vec3 right;
    vec3 up;
};

struct Plane {
    vec3 point;
    vec3 normal;
    vec3 color;
};

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    int type; // 0: opaque, 1: reflective, 2: refractive
};

struct Light {
    vec3 position;
    vec3 direction;
    vec3 color;
    float shininess;
    float cutoff; // if > 0.0 then spotlight else directional light
};

struct HitInfo {
    vec3 rayOrigin;
    vec3 rayDir;
    float t;
    vec3 baseColor;
    int inside; // 1 if inside the sphere, 0 otherwise
    vec3 hitPoint;
    vec3 normal;
    int type; // 0: diffuse, 1: reflective, 2: refractive
};

const int TYPE_DIFFUSE = 0;
const int TYPE_REFLECTIVE = 1;
const int TYPE_REFRACTIVE = 2;

const int MAX_SPHERES = 16;
const int MAX_LIGHTS = 4;
const int MAX_DEPTH = 5;
const float EPS = 0.001;
const float INF = 1e9;
const float IOR = 1.5;

in vec2 vUV;
out vec4 FragColor;

uniform float uTime;
uniform ivec2 uResolution; // width and height of canvas


uniform Camera cam;
uniform Sphere uSpheres[MAX_SPHERES];
uniform int uNumSpheres;

uniform Light uLights[MAX_LIGHTS];
uniform int uNumLights;

uniform Plane uPlane;

vec3 checkerboardColor(vec3 rgbColor, vec3 hitPoint) {
    // Checkerboard pattern
    float scaleParameter = 2.0;
    float checkerboard = 0.0;
    if (hitPoint.x < 0.0) {
    checkerboard += floor((0.5 - hitPoint.x) / scaleParameter);
    }
    else {
    checkerboard += floor(hitPoint.x / scaleParameter);
    }
    if (hitPoint.z < 0.0) {
    checkerboard += floor((0.5 - hitPoint.z) / scaleParameter);
    }
    else {
    checkerboard += floor(hitPoint.z / scaleParameter);
    }
    checkerboard = (checkerboard * 0.5) - float(int(checkerboard * 0.5));
    checkerboard *= 2.0;
    if (checkerboard > 0.5) {
    return 0.5 * rgbColor;
    }
    return rgbColor;
}

vec3 backgroundColor(vec3 dir) {
    float t = 0.5 * (dir.y + 1.0);
    return mix(vec3(0.1, 0.1, 0.15), vec3(0.5, 0.7, 1.0), clamp(t, 0.0, 1.0));
}

bool intersectPlane(vec3 rayOrigin, vec3 rayDir, out HitInfo hit) {
    vec3 n = normalize(uPlane.normal);
    float denom = dot(rayDir, n);
    if (abs(denom) < EPS) {
        return false;
    }

    float t = dot(uPlane.point - rayOrigin, n) / denom;
    if (t < EPS) {
        return false;
    }

    hit.rayOrigin = rayOrigin;
    hit.rayDir = rayDir;
    hit.t = t;
    hit.hitPoint = rayOrigin + rayDir * t;
    hit.normal = n;
    hit.baseColor = checkerboardColor(uPlane.color, hit.hitPoint);
    hit.type = TYPE_DIFFUSE;
    hit.inside = 0;
    return true;
}

bool intersectSphere(vec3 rayOrigin, vec3 rayDir, Sphere sphere, out HitInfo hit) {
    vec3 oc = rayOrigin - sphere.center;
    float b = dot(oc, rayDir);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float disc = b * b - c;
    if (disc < 0.0) {
        return false;
    }

    float sqrtDisc = sqrt(disc);
    float t0 = -b - sqrtDisc;
    float t1 = -b + sqrtDisc;
    float t = t0;
    if (t < EPS) {
        t = t1;
    }
    if (t < EPS) {
        return false;
    }

    hit.rayOrigin = rayOrigin;
    hit.rayDir = rayDir;
    hit.t = t;
    hit.hitPoint = rayOrigin + rayDir * t;

    vec3 normal = (hit.hitPoint - sphere.center) / sphere.radius;
    int inside = 0;
    if (dot(normal, rayDir) > 0.0) {
        normal = -normal;
        inside = 1;
    }

    hit.normal = normal;
    hit.baseColor = sphere.color;
    hit.type = sphere.type;
    hit.inside = inside;
    return true;
}

HitInfo intersectScene(vec3 rayOrigin, vec3 rayDir) {
    HitInfo bestHit;
    bestHit.t = INF;
    bestHit.baseColor = vec3(0.0);
    bestHit.type = TYPE_DIFFUSE;
    bestHit.rayOrigin = rayOrigin;
    bestHit.rayDir = rayDir;

    HitInfo candidate;
    if (intersectPlane(rayOrigin, rayDir, candidate)) {
        if (candidate.t < bestHit.t) {
            bestHit = candidate;
        }
    }

    for (int i = 0; i < MAX_SPHERES; ++i) {
        if (i >= uNumSpheres) {
            break;
        }
        if (intersectSphere(rayOrigin, rayDir, uSpheres[i], candidate)) {
            if (candidate.t < bestHit.t) {
                bestHit = candidate;
            }
        }
    }

    return bestHit;
}

bool shadowHit(vec3 origin, vec3 dir, float maxDist) {
    HitInfo h = intersectScene(origin, dir);
    return h.t < maxDist;
}

vec3 phongLighting(const HitInfo hit) {
    vec3 N = normalize(hit.normal);
    vec3 V = normalize(-hit.rayDir);

    vec3 ambient = 0.05 * hit.baseColor;
    vec3 color = ambient;

    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (i >= uNumLights) {
            break;
        }
        Light light = uLights[i];

        vec3 L;
        float maxDist = INF;
        bool isSpot = light.cutoff > 0.0;
        if (isSpot) {
            vec3 toLight = light.position - hit.hitPoint;
            maxDist = length(toLight);
            L = toLight / maxDist;

            float spot = dot(normalize(light.direction), -L);
            if (spot < light.cutoff) {
                continue;
            }
        } else {
            L = normalize(-light.direction);
        }

        vec3 shadowOrigin = hit.hitPoint + N * EPS;
        if (shadowHit(shadowOrigin, L, maxDist - EPS)) {
            continue;
        }

        float diff = max(dot(N, L), 0.0);
        vec3 diffuse = diff * hit.baseColor * light.color;

        vec3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0), light.shininess);
        vec3 specular = spec * light.color;

        color += diffuse + specular;
    }

    return color;
}

/* calculates color based on tracing rays iteratively */
vec3 calcColor(vec3 rayOrigin, vec3 rayDir) {
    vec3 radiance = vec3(0.0);
    vec3 throughput = vec3(1.0);
    vec3 origin = rayOrigin;
    vec3 dir = rayDir;

    for (int depth = 0; depth < MAX_DEPTH; ++depth) {
        HitInfo hit = intersectScene(origin, dir);
        if (hit.t >= INF * 0.5) {
            radiance += throughput * backgroundColor(dir);
            break;
        }

        if (hit.type == TYPE_REFLECTIVE) {
            vec3 reflectedDir = reflect(dir, normalize(hit.normal));
            origin = hit.hitPoint + reflectedDir * EPS;
            dir = reflectedDir;
            throughput *= hit.baseColor;
            continue;
        }

        if (hit.type == TYPE_REFRACTIVE) {
            float eta = 1.0 / IOR;
            vec3 n = normalize(hit.normal);
            if (hit.inside == 1) {
                eta = IOR;
                n = -n;
            }
            vec3 refractedDir = refract(dir, n, eta);
            if (length(refractedDir) < 0.001) {
                refractedDir = reflect(dir, n);
            }
            origin = hit.hitPoint + refractedDir * EPS;
            dir = refractedDir;
            throughput *= hit.baseColor;
            continue;
        }

        radiance += throughput * phongLighting(hit);
        break;
    }

    return radiance;
}

/* scales UV coordinates based on resolution
 * uv given uv are [0, 1] range
 * returns new coordinates where y range [-1, 1] and x scales according to window resolution
 */
vec2 scaleUV(vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0;
    float aspect = float(uResolution.x) / float(uResolution.y);
    ndc.x *= aspect;
    return ndc;
}

void main() {
    vec2 uv = scaleUV(vUV);
    vec3 rayDir = normalize(cam.forward + uv.x * cam.right + uv.y * cam.up);

    vec3 color = calcColor(cam.pos, rayDir);
    FragColor = vec4(color, 1.0);
}
