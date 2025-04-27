#include "CollisionDetection.h"
#include "ShipController.h"
#include "Asteroid.h"
#include <iostream>
#include <limits>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

CollisionDetection::CollisionDetection() :
    shipController(nullptr),
    asteroidManager(nullptr),
    shipRadius(0.0f),
    collisionDetected(false),
    timeSinceLastCollision(0.0f),
    collisionCooldown(1.0f)
{
}

void CollisionDetection::initialize(ShipController* ship, AsteroidManager* asteroids, float shipCollisionRadius) {
    shipController = ship;
    asteroidManager = asteroids;
    shipRadius = shipCollisionRadius * 0.4f;
}

void CollisionDetection::update(float deltaTime) {
    if (collisionDetected) {
        timeSinceLastCollision += deltaTime;
        if (timeSinceLastCollision >= collisionCooldown) {
            collisionDetected = false;
        }
    }

    if (!collisionDetected) {
        checkShipAsteroidCollisions();
    }
}

bool CollisionDetection::checkShipAsteroidCollisions() {
    if (!shipController || !asteroidManager) return false;

    glm::vec3 shipPosition = shipController->getPosition();

    BoundingSphere shipSphere;
    shipSphere.center = shipPosition;
    shipSphere.radius = shipRadius;

    const std::vector<Asteroid>& asteroids = asteroidManager->getAsteroids();
    Aabb asteroidBBox = asteroidManager->getBoundingBox();
    float baseMeshRadius = glm::length(asteroidBBox.max - asteroidBBox.min) * 0.5f;

    for (const auto& asteroid : asteroids) {
        BoundingSphere asteroidSphere;
        asteroidSphere.center = asteroid.position;
        float avgScale = (asteroid.scale.x + asteroid.scale.y + asteroid.scale.z) / 3.0f;
        asteroidSphere.radius = baseMeshRadius * avgScale * 0.8f;

        if (CollisionDetection::sphereToSphereCollision(shipSphere, asteroidSphere)) {
            collisionDetected = true;
            timeSinceLastCollision = 0.0f;

            if (onCollision) {
                onCollision(asteroid);
            }

            return true;
        }
    }

    return false;
}

bool CollisionDetection::sphereToSphereCollision(const BoundingSphere& a, const BoundingSphere& b) {
    float distance = glm::length(a.center - b.center);
    return distance < (a.radius + b.radius);
}

Aabb CollisionDetection::getBoundingBox() const {
    if (asteroidManager) {
        return asteroidManager->getBoundingBox();
    }
    return Aabb();
}
